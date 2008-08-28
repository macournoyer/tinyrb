#include "tinyrb.h"

/* Based on hashtable http://www.cl.cam.ac.uk/~cwc22/hashtable/
 * Copyright (C) 2004 Christopher Clark <firstname.lastname@cl.cam.ac.uk>
 * Credit for primes table: Aaron Krowne
 *  http://br.endernet.org/~akrowne/
 *  http://planetmath.org/encyclopedia/GoodHashTablePrimes.html */

static const u_int primes[] = {
  53, 97, 193, 389,
  769, 1543, 3079, 6151,
  12289, 24593, 49157, 98317,
  196613, 393241, 786433, 1572869,
  3145739, 6291469, 12582917, 25165843,
  50331653, 100663319, 201326611, 402653189,
  805306457, 1610612741
};
const u_int prime_table_length = sizeof(primes)/sizeof(primes[0]);
const float max_load_factor = 0.65;

#define HASH_MINSIZE    1024 /* TODO test and tweak */
#define HASH_INDEX(v,l) ((v) % (l))
#define TR_OBJ_HASH(v)  TR_FIX(tr_send2(vm, v, "hash", 0))

#define freekey(X) tr_free(X)

static u_int tr_string_hash(char *str)
{
  unsigned long  hash = 5381;
  int            c;
  
  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  
  return hash;
}

static u_int tr_hashcode(VM, OBJ v)
{
  u_int i;
  
  switch (TR_TYPE(v)) {
    case TR_STRING:  i = tr_string_hash(TR_STR(v)); break;
    case TR_FIXNUM:  i = TR_FIX(v); break;
    case TR_SYMBOL:
    case TR_SPECIAL: i = v; break;
    default:         TR_OBJ_HASH(v);
  }
  
  /* Aim to protect against poor hash functions by adding logic here
   * - logic taken from java 1.4 hash source */
  i += ~(i << 9);
  i ^=  ((i >> 14) | (i << 18)); /* >>> */
  i +=  (i << 4);
  i ^=  ((i >> 10) | (i << 22)); /* >>> */
  
  return i;
}

static int tr_hash_keys_compare(VM, OBJ key1, OBJ key2)
{
  switch (TR_TYPE(key1)) {
    case TR_STRING: return strcmp((char *) key1, (char *) key2) == 0;
    case TR_FIXNUM: return TR_FIX(key1) == TR_FIX(key2);
    case TR_SYMBOL: return key1 == key2;
  }
  return TR_OBJ_HASH(key1) == TR_OBJ_HASH(key2);
}

tr_hash *tr_hash_struct(VM)
{
  tr_hash *h;
  u_int    pindex, size = primes[0];
  
  /* Enforce size as prime */
  for (pindex=0; pindex < prime_table_length; pindex++) {
    if (primes[pindex] > HASH_MINSIZE) {
      size = primes[pindex];
      break;
    }
  }
  
  h = (tr_hash *) tr_malloc(sizeof(tr_hash));
    
  h->table = (tr_hash_entry **) tr_malloc(sizeof(tr_hash_entry*) * size);
  
  memset(h->table, 0, size * sizeof(tr_hash_entry *));
  h->type            = TR_HASH;
  h->tablelength     = size;
  h->primeindex      = pindex;
  h->hash_entrycount = 0;
  h->loadlimit       = (u_int) ceil(size * max_load_factor);
  
  return h;
}

OBJ tr_hash_new(VM)
{
  tr_hash *h = tr_hash_struct(vm);
  
  tr_obj_init(vm, TR_HASH, (OBJ) h, tr_const_get(vm, "Hash"));

  return (OBJ) h;
}

static int tr_hash_expand(tr_hash *h)
{
  /* Double the size of the table to accomodate more entries */
  tr_hash_entry **newtable;
  tr_hash_entry  *e;
  tr_hash_entry **pE;
  u_int           newsize, i, index;
  
  /* Check we're not hitting max capacity */
  if (h->primeindex == (prime_table_length - 1))
    return 0;
  
  newsize  = primes[++(h->primeindex)];
  newtable = (tr_hash_entry **) tr_malloc(sizeof(tr_hash_entry*) * newsize);
  
  if (NULL != newtable) {
    memset(newtable, 0, newsize * sizeof(tr_hash_entry *));
    /* This algorithm is not 'stable'. ie. it reverses the list
     * when it transfers entries between the tables */
    for (i = 0; i < h->tablelength; i++) {
      while (NULL != (e = h->table[i])) {
        h->table[i] = e->next;
        index = HASH_INDEX(newsize,e->h);
        e->next = newtable[index];
        newtable[index] = e;
      }
    }
    tr_free(h->table);
    h->table = newtable;
  }
  /* Plan B: realloc instead */
  else {
    newtable = (tr_hash_entry **) tr_realloc(h->table, newsize * sizeof(tr_hash_entry *));
    if (NULL == newtable) {
      (h->primeindex)--;
      return 0;
    }
    
    h->table = newtable;
    memset(newtable[h->tablelength], 0, newsize - h->tablelength);
    
    for (i = 0; i < h->tablelength; i++) {
      for (pE = &(newtable[i]), e = *pE; e != NULL; e = *pE) {
        index = HASH_INDEX(newsize,e->h);
        
        if (index == i) {
          pE              = &(e->next);
        } else {
          *pE             = e->next;
          e->next         = newtable[index];
          newtable[index] = e;
        }
      }
    }
  }
  h->tablelength = newsize;
  h->loadlimit   = (u_int) ceil(newsize * max_load_factor);
  
  return -1;
}

OBJ tr_hash_count(VM, OBJ h)
{
  return tr_fixnum_new(vm, TR_CHASH(h)->hash_entrycount);
}

OBJ tr_hash_set(VM, OBJ o, OBJ k, OBJ v)
{
  /* This method allows duplicate keys - but they shouldn't be used */
  tr_hash       *h = TR_CHASH(o);
  u_int          index;
  tr_hash_entry *e;
  
  if (++(h->hash_entrycount) > h->loadlimit) {
    /* Ignore the return value. If expand fails, we should
     * still try cramming just this value into the existing table
     * -- we may not have memory for a larger table, but one more
     * element may be ok. Next time we insert, we'll try expanding again.*/
    tr_hash_expand(h);
  }
  
  e = (tr_hash_entry *) tr_malloc(sizeof(tr_hash_entry));
  if (NULL == e) {
    --(h->hash_entrycount);
    return TR_NIL;
  } /*oom*/
  
  e->h    = tr_hashcode(vm, k);
  index   = HASH_INDEX(h->tablelength, e->h);
  e->k    = k;
  e->v    = v;
  e->next = h->table[index];
  h->table[index] = e;
  
  return v;
}

OBJ tr_hash_get(VM, OBJ o, OBJ k)
{
  tr_hash       *h = TR_CHASH(o);
  tr_hash_entry *e;
  u_int          hashvalue, index;
  
  hashvalue = tr_hashcode(vm, k);
  index     = HASH_INDEX(h->tablelength, hashvalue);
  e         = h->table[index];
  
  while (NULL != e) {
    /* Check hash value to short circuit heavier comparison */
    if ((hashvalue == e->h) && (tr_hash_keys_compare(vm, k, e->k)))
      return e->v;
    e = e->next;
  }
  
  return TR_NIL;
}

OBJ tr_hash_delete(VM, OBJ o, OBJ k)
{
  /* TODO: consider compacting the table when the load factor drops enough,
   *       or provide a 'compact' method. */

  tr_hash        *h = TR_CHASH(o);
  tr_hash_entry  *e;
  tr_hash_entry **pE;
  OBJ             v;
  u_int           hashvalue, index;

  hashvalue = tr_hashcode(vm, k);
  index     = HASH_INDEX(h->tablelength, tr_hashcode(vm, k));
  pE        = &(h->table[index]);
  e         = *pE;
  
  while (NULL != e) {
    /* Check hash value to short circuit heavier comparison */
    if ((hashvalue == e->h) && (tr_hash_keys_compare(vm, k, e->k))) {
      *pE = e->next;
      h->hash_entrycount--;
      v = e->v;
      freekey(e->k);
      tr_free(e);
      return v;
    }
    pE = &(e->next);
    e  = e->next;
  }
  
  return TR_NIL;
}

static OBJ tr_hash_keys(VM, OBJ self)
{
  tr_hash        *h = TR_CHASH(self);
  tr_hash_entry  *e;
  tr_hash_entry **table = h->table;
  u_int           i;
  OBJ             ary = tr_array_new(vm);
  
  for (i = 0; i <= h->tablelength; i++) {
    e = table[i];
    while (NULL != e) {
      tr_array_push(vm, ary, e->k);
      e = e->next;
    }
  }
  
  return ary;
}

void tr_hash_init(VM)
{
  OBJ class = tr_class_new(vm, "Hash", tr_const_get(vm, "Object"));
  
  tr_def(vm, class, "[]", tr_hash_get, 1);
  tr_def(vm, class, "[]=", tr_hash_set, 2);
  tr_def(vm, class, "delete", tr_hash_delete, 1);
  tr_def(vm, class, "count", tr_hash_count, 0);
  tr_def(vm, class, "size", tr_hash_count, 0);
  tr_def(vm, class, "keys", tr_hash_keys, 0);
}
