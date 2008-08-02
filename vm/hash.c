#include "tinyrb.h"

/* Based on hashtable http://www.cl.cam.ac.uk/~cwc22/hashtable/
   Copyright (C) 2004 Christopher Clark <firstname.lastname@cl.cam.ac.uk>
   Credit for primes table: Aaron Krowne
   http://br.endernet.org/~akrowne/
   http://planetmath.org/encyclopedia/GoodHashTablePrimes.html */
static const unsigned int primes[] = {
53, 97, 193, 389,
769, 1543, 3079, 6151,
12289, 24593, 49157, 98317,
196613, 393241, 786433, 1572869,
3145739, 6291469, 12582917, 25165843,
50331653, 100663319, 201326611, 402653189,
805306457, 1610612741
};
const unsigned int prime_table_length = sizeof(primes)/sizeof(primes[0]);
const float max_load_factor = 0.65;

#define freekey(X) free(X)

static inline unsigned int
indexFor(unsigned int tablelength, unsigned int tr_hashvalue) {
    return (tr_hashvalue % tablelength);
};

tr_hash *tr_hash_create(unsigned int minsize,
                 unsigned int (*tr_hashf) (void*),
                 int (*eqf) (void*,void*))
{
    tr_hash *h;
    unsigned int pindex, size = primes[0];
    /* Check requested hash isn't too large */
    if (minsize > (1u << 30)) return NULL;
    /* Enforce size as prime */
    for (pindex=0; pindex < prime_table_length; pindex++) {
        if (primes[pindex] > minsize) { size = primes[pindex]; break; }
    }
    h = (tr_hash *)malloc(sizeof(tr_hash));
    if (NULL == h) return NULL; /*oom*/
    h->table = (tr_hash_entry **)malloc(sizeof(tr_hash_entry*) * size);
    if (NULL == h->table) { free(h); return NULL; } /*oom*/
    memset(h->table, 0, size * sizeof(tr_hash_entry *));
    h->tablelength  = size;
    h->primeindex   = pindex;
    h->hash_entrycount   = 0;
    h->hashfn       = tr_hashf;
    h->eqfn         = eqf;
    h->loadlimit    = (unsigned int) ceil(size * max_load_factor);
    return h;
}

unsigned int tr_hashcode(tr_hash *h, void *k)
{
    /* Aim to protect against poor tr_hash functions by adding logic here
     * - logic taken from java 1.4 tr_hash source */
    unsigned int i = h->hashfn(k);
    i += ~(i << 9);
    i ^=  ((i >> 14) | (i << 18)); /* >>> */
    i +=  (i << 4);
    i ^=  ((i >> 10) | (i << 22)); /* >>> */
    return i;
}

static int tr_hash_expand(tr_hash *h)
{
    /* Double the size of the table to accomodate more entries */
    tr_hash_entry **newtable;
    tr_hash_entry *e;
    tr_hash_entry **pE;
    unsigned int newsize, i, index;
    /* Check we're not hitting max capacity */
    if (h->primeindex == (prime_table_length - 1)) return 0;
    newsize = primes[++(h->primeindex)];

    newtable = (tr_hash_entry **)malloc(sizeof(tr_hash_entry*) * newsize);
    if (NULL != newtable)
    {
        memset(newtable, 0, newsize * sizeof(tr_hash_entry *));
        /* This algorithm is not 'stable'. ie. it reverses the list
         * when it transfers entries between the tables */
        for (i = 0; i < h->tablelength; i++) {
            while (NULL != (e = h->table[i])) {
                h->table[i] = e->next;
                index = indexFor(newsize,e->h);
                e->next = newtable[index];
                newtable[index] = e;
            }
        }
        free(h->table);
        h->table = newtable;
    }
    /* Plan B: realloc instead */
    else 
    {
        newtable = (tr_hash_entry **)
                   realloc(h->table, newsize * sizeof(tr_hash_entry *));
        if (NULL == newtable) { (h->primeindex)--; return 0; }
        h->table = newtable;
        memset(newtable[h->tablelength], 0, newsize - h->tablelength);
        for (i = 0; i < h->tablelength; i++) {
            for (pE = &(newtable[i]), e = *pE; e != NULL; e = *pE) {
                index = indexFor(newsize,e->h);
                if (index == i)
                {
                    pE = &(e->next);
                }
                else
                {
                    *pE = e->next;
                    e->next = newtable[index];
                    newtable[index] = e;
                }
            }
        }
    }
    h->tablelength = newsize;
    h->loadlimit   = (unsigned int) ceil(newsize * max_load_factor);
    return -1;
}

unsigned int tr_hash_count(tr_hash *h)
{
    return h->hash_entrycount;
}

int tr_hash_insert(tr_hash *h, void *k, void *v)
{
    /* This method allows duplicate keys - but they shouldn't be used */
    unsigned int index;
    tr_hash_entry *e;
    if (++(h->hash_entrycount) > h->loadlimit)
    {
        /* Ignore the return value. If expand fails, we should
         * still try cramming just this value into the existing table
         * -- we may not have memory for a larger table, but one more
         * element may be ok. Next time we insert, we'll try expanding again.*/
        tr_hash_expand(h);
    }
    e = (tr_hash_entry *)malloc(sizeof(tr_hash_entry));
    if (NULL == e) { --(h->hash_entrycount); return 0; } /*oom*/
    e->h = tr_hashcode(h,k);
    index = indexFor(h->tablelength,e->h);
    e->k = k;
    e->v = v;
    e->next = h->table[index];
    h->table[index] = e;
    return -1;
}

void *tr_hash_search(tr_hash *h, void *k)
{
    tr_hash_entry *e;
    unsigned int tr_hashvalue, index;
    tr_hashvalue = tr_hashcode(h,k);
    index = indexFor(h->tablelength,tr_hashvalue);
    e = h->table[index];
    while (NULL != e)
    {
        /* Check tr_hash value to short circuit heavier comparison */
        if ((tr_hashvalue == e->h) && (h->eqfn(k, e->k))) return e->v;
        e = e->next;
    }
    return NULL;
}

void *tr_hash_remove(tr_hash *h, void *k)
{
    /* TODO: consider compacting the table when the load factor drops enough,
     *       or provide a 'compact' method. */

    tr_hash_entry *e;
    tr_hash_entry **pE;
    void *v;
    unsigned int tr_hashvalue, index;

    tr_hashvalue = tr_hashcode(h,k);
    index = indexFor(h->tablelength,tr_hashcode(h,k));
    pE = &(h->table[index]);
    e = *pE;
    while (NULL != e)
    {
        /* Check tr_hash value to short circuit heavier comparison */
        if ((tr_hashvalue == e->h) && (h->eqfn(k, e->k)))
        {
            *pE = e->next;
            h->hash_entrycount--;
            v = e->v;
            freekey(e->k);
            free(e);
            return v;
        }
        pE = &(e->next);
        e = e->next;
    }
    return NULL;
}

void tr_hash_destroy(tr_hash *h, int free_values)
{
    unsigned int i;
    tr_hash_entry *e, *f;
    tr_hash_entry **table = h->table;
    if (free_values)
    {
        for (i = 0; i < h->tablelength; i++)
        {
            e = table[i];
            while (NULL != e)
            { f = e; e = e->next; freekey(f->k); free(f->v); free(f); }
        }
    }
    else
    {
        for (i = 0; i < h->tablelength; i++)
        {
            e = table[i];
            while (NULL != e)
            { f = e; e = e->next; freekey(f->k); free(f); }
        }
    }
    free(h->table);
    free(h);
}

static unsigned int hash_from_str_key_fn(void *k)
{
  char * str = (char *) k;
  unsigned long tr_hash = 5381;
  int c;

  while (c = *str++)
    tr_hash = ((tr_hash << 5) + tr_hash) + c; /* tr_hash * 33 + c */

  return tr_hash;
}

static int str_keys_equal_fn(void *key1, void *key2)
{
  return strcmp((char *) key1, (char *) key2) == 0;
}

tr_hash *tr_hash_new(unsigned int minsize)
{
  return tr_hash_create(minsize, hash_from_str_key_fn, str_keys_equal_fn);
}
