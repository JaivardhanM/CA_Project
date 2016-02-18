/* cache.c - cache module routines */

/* SimpleScalar(TM) Tool Suite
 * Copyright (C) 1994-2003 by Todd M. Austin, Ph.D. and SimpleScalar, LLC.
 * All Rights Reserved. 
 * 
 * THIS IS A LEGAL DOCUMENT, BY USING SIMPLESCALAR,
 * YOU ARE AGREEING TO THESE TERMS AND CONDITIONS.
 * 
 * No portion of this work may be used by any commercial entity, or for any
 * commercial purpose, without the prior, written permission of SimpleScalar,
 * LLC (info@simplescalar.com). Nonprofit and noncommercial use is permitted
 * as described below.
 * 
 * 1. SimpleScalar is provided AS IS, with no warranty of any kind, express
 * or implied. The user of the program accepts full responsibility for the
 * application of the program and the use of any results.
 * 
 * 2. Nonprofit and noncommercial use is encouraged. SimpleScalar may be
 * downloaded, compiled, executed, copied, and modified solely for nonprofit,
 * educational, noncommercial research, and noncommercial scholarship
 * purposes provided that this notice in its entirety accompanies all copies.
 * Copies of the modified software can be delivered to persons who use it
 * solely for nonprofit, educational, noncommercial research, and
 * noncommercial scholarship purposes provided that this notice in its
 * entirety accompanies all copies.
 * 
 * 3. ALL COMMERCIAL USE, AND ALL USE BY FOR PROFIT ENTITIES, IS EXPRESSLY
 * PROHIBITED WITHOUT A LICENSE FROM SIMPLESCALAR, LLC (info@simplescalar.com).
 * 
 * 4. No nonprofit user may place any restrictions on the use of this software,
 * including as modified by the user, by any other authorized user.
 * 
 * 5. Noncommercial and nonprofit users may distribute copies of SimpleScalar
 * in compiled or executable form as set forth in Section 2, provided that
 * either: (A) it is accompanied by the corresponding machine-readable source
 * code, or (B) it is accompanied by a written offer, with no time limit, to
 * give anyone a machine-readable copy of the corresponding source code in
 * return for reimbursement of the cost of distribution. This written offer
 * must permit verbatim duplication by anyone, or (C) it is distributed by
 * someone who received only the executable form, and is accompanied by a
 * copy of the written offer of source code.
 * 
 * 6. SimpleScalar was developed by Todd M. Austin, Ph.D. The tool suite is
 * currently maintained by SimpleScalar LLC (info@simplescalar.com). US Mail:
 * 2395 Timbercrest Court, Ann Arbor, MI 48105.
 * 
 * Copyright (C) 1994-2003 by Todd M. Austin, Ph.D. and SimpleScalar, LLC.
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "cache.h"
#include "sim.h"

/* cache access macros */
#define CACHE_TAG(cp, addr)	((addr) >> (cp)->tag_shift)
#define CACHE_SET(cp, addr)	(((addr) >> (cp)->set_shift) & (cp)->set_mask)
#define CACHE_BLK(cp, addr)	((addr) & (cp)->blk_mask)
#define CACHE_TAGSET(cp, addr)	((addr) & (cp)->tagset_mask)

/* extract/reconstruct a block address */
#define CACHE_BADDR(cp, addr)	((addr) & ~(cp)->blk_mask)
#define CACHE_MK_BADDR(cp, tag, set)					\
  (((tag) << (cp)->tag_shift)|((set) << (cp)->set_shift))

/* index an array of cache blocks, non-trivial due to variable length blocks */
#define CACHE_BINDEX(cp, blks, i)					\
  ((struct cache_blk_t *)(((char *)(blks)) +				\
			  (i)*(sizeof(struct cache_blk_t) +		\
			       ((cp)->balloc				\
				? (cp)->bsize*sizeof(byte_t) : 0))))

/* cache data block accessor, type parameterized */
#define __CACHE_ACCESS(type, data, bofs)				\
  (*((type *)(((char *)data) + (bofs))))

/* cache data block accessors, by type */
#define CACHE_DOUBLE(data, bofs)  __CACHE_ACCESS(double, data, bofs)
#define CACHE_FLOAT(data, bofs)	  __CACHE_ACCESS(float, data, bofs)
#define CACHE_WORD(data, bofs)	  __CACHE_ACCESS(unsigned int, data, bofs)
#define CACHE_HALF(data, bofs)	  __CACHE_ACCESS(unsigned short, data, bofs)
#define CACHE_BYTE(data, bofs)	  __CACHE_ACCESS(unsigned char, data, bofs)

/* cache block hashing macros, this macro is used to index into a cache
   set hash table (to find the correct block on N in an N-way cache), the
   cache set index function is CACHE_SET, defined above */
#define CACHE_HASH(cp, key)						\
  (((key >> 24) ^ (key >> 16) ^ (key >> 8) ^ key) & ((cp)->hsize-1))

/* copy data out of a cache block to buffer indicated by argument pointer p */
#define CACHE_BCOPY(cmd, blk, bofs, p, nbytes)	\
  if (cmd == Read)							\
    {									\
      switch (nbytes) {							\
      case 1:								\
	*((byte_t *)p) = CACHE_BYTE(&blk->data[0], bofs); break;	\
      case 2:								\
	*((half_t *)p) = CACHE_HALF(&blk->data[0], bofs); break;	\
      case 4:								\
	*((word_t *)p) = CACHE_WORD(&blk->data[0], bofs); break;	\
      default:								\
	{ /* >= 8, power of two, fits in block */			\
	  int words = nbytes >> 2;					\
	  while (words-- > 0)						\
	    {								\
	      *((word_t *)p) = CACHE_WORD(&blk->data[0], bofs);	\
	      p += 4; bofs += 4;					\
	    }\
	}\
      }\
    }\
  else /* cmd == Write */						\
    {									\
      switch (nbytes) {							\
      case 1:								\
	CACHE_BYTE(&blk->data[0], bofs) = *((byte_t *)p); break;	\
      case 2:								\
        CACHE_HALF(&blk->data[0], bofs) = *((half_t *)p); break;	\
      case 4:								\
	CACHE_WORD(&blk->data[0], bofs) = *((word_t *)p); break;	\
      default:								\
	{ /* >= 8, power of two, fits in block */			\
	  int words = nbytes >> 2;					\
	  while (words-- > 0)						\
	    {								\
	      CACHE_WORD(&blk->data[0], bofs) = *((word_t *)p);		\
	      p += 4; bofs += 4;					\
	    }\
	}\
    }\
  }

counter_t Smaller_Cache_Access_Cnt = 0;
counter_t Larger_Cache_Access_Cnt = 0;

/* bound sqword_t/dfloat_t to positive int */
#define BOUND_POS(N)		((int)(MIN(MAX(0, (N)), 2147483647)))

/* unlink BLK from the hash table bucket chain in SET */
static void
unlink_htab_ent(struct cache_t *cp,		/* cache to update */
		struct cache_set_t *set,	/* set containing bkt chain */
		struct cache_blk_t *blk)	/* block to unlink */
{
  struct cache_blk_t *prev, *ent;
  int index = CACHE_HASH(cp, blk->tag);

  /* locate the block in the hash table bucket chain */
  for (prev=NULL,ent=set->hash[index];
       ent;
       prev=ent,ent=ent->hash_next)
    {
      if (ent == blk)
	break;
    }
  assert(ent);

  /* unlink the block from the hash table bucket chain */
  if (!prev)
    {
      /* head of hash bucket list */
      set->hash[index] = ent->hash_next;
    }
  else
    {
      /* middle or end of hash bucket list */
      prev->hash_next = ent->hash_next;
    }
  ent->hash_next = NULL;
}

/* insert BLK onto the head of the hash table bucket chain in SET */
static void
link_htab_ent(struct cache_t *cp,		/* cache to update */
	      struct cache_set_t *set,		/* set containing bkt chain */
	      struct cache_blk_t *blk)		/* block to insert */
{
  int index = CACHE_HASH(cp, blk->tag);

  /* insert block onto the head of the bucket chain */
  blk->hash_next = set->hash[index];
  set->hash[index] = blk;
}

/* where to insert a block onto the ordered way chain */
enum list_loc_t { Head, Tail };

/* insert BLK into the order way chain in SET at location WHERE */
static void
update_way_list(struct cache_set_t *set,	/* set contained way chain */
		struct cache_blk_t *blk,	/* block to insert */
		enum list_loc_t where,  /* insert location */
    struct cache_t *cp)     /* cache to update -AJS */		
{
  /* unlink entry from the way list */
  if (!blk->way_prev && !blk->way_next)
    {
      /* only one entry in list (direct-mapped), no action */
      assert(set->way_head == blk && set->way_tail == blk);
      /* Head/Tail order already */
      return;
    }
  /* else, more than one element in the list */
  else if (!blk->way_prev)
    {
      assert(set->way_head == blk && set->way_tail != blk);
      if (where == Head)
	{
	  /* already there */
	  return;
	}
      /* else, move to tail */
      set->way_head = blk->way_next;
      blk->way_next->way_prev = NULL;
    }
  else if (!blk->way_next)
    {
      /* end of list (and not front of list) */
      assert(set->way_head != blk && set->way_tail == blk);
      if (where == Tail)
	{
	  /* already there */
	  return;
	}
      set->way_tail = blk->way_prev;
      blk->way_prev->way_next = NULL;
    }
  else
    {
      /* middle of list (and not front or end of list) */
      assert(set->way_head != blk && set->way_tail != blk);
      blk->way_prev->way_next = blk->way_next;
      blk->way_next->way_prev = blk->way_prev;
    }

  /* link BLK back into the list */
  if (where == Head)
    {
      /* link to the head of the way list */
      blk->way_next = set->way_head;
      blk->way_prev = NULL;
      set->way_head->way_prev = blk;
      set->way_head = blk;
    }
  else if (where == Tail)
    {
      /* link to the tail of the way list */
      blk->way_prev = set->way_tail;
      blk->way_next = NULL;
      set->way_tail->way_next = blk;
      set->way_tail = blk;
    }
  else
    panic("bogus WHERE designator");
	
	  
  if (cp && TournamentLRU == cp->policy)
    cache_reconfigure_set(cp,set);
}

//rv
// MNM table init funtion
void mnm_init(struct cache_t *cp)
{ int i =0;
  for(i =0; i < MNM_TABLE_SIZE ; i++)
    cp->mnm_table[i] = 0;

}



/* create and initialize a general cache structure */
struct cache_t *			/* pointer to cache created */
cache_create(char *name,		/* name of the cache */
	     int nsets,			/* total number of sets in cache */
	     int bsize,			/* block (line) size of cache */
	     int balloc,		/* allocate data space for blocks? */
	     int usize,			/* size of user data to alloc w/blks */
	     int assoc,			/* associativity of cache */
	     enum cache_policy policy,	/* replacement policy w/in sets */
	     /* block access function, see description w/in struct cache def */
	     unsigned int (*blk_access_fn)(enum mem_cmd cmd,
					   md_addr_t baddr, int bsize,
					   struct cache_blk_t *blk,
					   tick_t now),
        unsigned int hit_latency,/* latency in cycles for a hit */
        unsigned int max_miss_saturation,
        unsigned int tournament_length,
        unsigned int accesses_bw_tournaments,
        unsigned int hits_for_win)
{
  struct cache_t *cp;
  struct cache_blk_t *blk;
  int i, j, bindex;
  

  /* check all cache parameters */
  if (nsets <= 0)
    fatal("cache size (in sets) `%d' must be non-zero", nsets);
  if ((nsets & (nsets-1)) != 0)
    fatal("cache size (in sets) `%d' is not a power of two", nsets);
  /* blocks must be at least one datum large, i.e., 8 bytes for SS */
  if (bsize < 8)
    fatal("cache block size (in bytes) `%d' must be 8 or greater", bsize);
  if ((bsize & (bsize-1)) != 0)
    fatal("cache block size (in bytes) `%d' must be a power of two", bsize);
  if (usize < 0)
    fatal("user data size (in bytes) `%d' must be a positive value", usize);
  if (assoc <= 0)
    fatal("cache associativity `%d' must be non-zero and positive", assoc);
  /* -AJS: Removed power of two constraint
    if ((assoc & (assoc-1)) != 0)
      fatal("cache associativity `%d' must be a power of two", assoc);
  */
  if (!blk_access_fn)
    fatal("must specify miss/replacement functions");

  /* allocate the cache structure */
  cp = (struct cache_t *)
    calloc(1, sizeof(struct cache_t) + (nsets-1)*sizeof(struct cache_set_t));
  if (!cp)
    fatal("out of virtual memory");

  //rv
  cp->isMNM=FALSE;
  //cp->Mnm_Counter_Max = (1<< (32 - cp->tag_shift - MNM_INDEX_BITS));
  cp->Mnm_Counter_Max = 8;
  mnm_init(cp);

  

  /* initialize user parameters */
  cp->name = mystrdup(name);
  cp->nsets = nsets;
  cp->bsize = bsize;
  cp->balloc = balloc;
  cp->usize = usize;
  cp->assoc = assoc;
  cp->policy = policy;
  cp->hit_latency = hit_latency;
  /* -AJS Initialize new parameters*/
  cp->data_cols = assoc;
  cp->tag_cols = assoc;
  cp->miss_saturation = 0;
  cp->MAX_MISS_SATURATION = max_miss_saturation;
  cp->TOURNAMENT_LENGTH = tournament_length;
  cp->ACCESSES_BW_TOURNAMENTS = accesses_bw_tournaments;
  cp->TOURNAMENT_HITS_FOR_WIN = hits_for_win;
  cp->accesses_since_tournament = 0;
  cp->tournament_status = T_NONE;
  cp->Current_Cache_Config = T_NONE;
  /* miss/replacement functions */
  cp->blk_access_fn = blk_access_fn;

  /* compute derived parameters */
  cp->hsize = CACHE_HIGHLY_ASSOC(cp) ? (assoc >> 2) : 0;
  /* -AJS No hashing for TournamentLRU */
  if (TournamentLRU == cp->policy)
  cp->hsize = 0;
  cp->blk_mask = bsize-1;
  cp->set_shift = log_base2(bsize);
  cp->set_mask = nsets-1;
  cp->tag_shift = cp->set_shift + log_base2(nsets);
  cp->tag_mask = (1 << (32 - cp->tag_shift))-1;
  cp->tagset_mask = ~cp->blk_mask;
  cp->bus_free = 0;

  /* print derived parameters during debug */
  debug("%s: cp->hsize     = %d", cp->name, cp->hsize);
  debug("%s: cp->blk_mask  = 0x%08x", cp->name, cp->blk_mask);
  debug("%s: cp->set_shift = %d", cp->name, cp->set_shift);
  debug("%s: cp->set_mask  = 0x%08x", cp->name, cp->set_mask);
  debug("%s: cp->tag_shift = %d", cp->name, cp->tag_shift);
  debug("%s: cp->tag_mask  = 0x%08x", cp->name, cp->tag_mask);

  /* initialize cache stats */
  cp->hits = 0;
  cp->misses = 0;
  cp->mnm_misses=0;
  cp->true_mnm_misses=0;
  cp->replacements = 0;
  cp->writebacks = 0;
  cp->invalidations = 0;
  cp->tournaments = 0;
  cp->reconfigurations = 0;
  /* blow away the last block accessed */
  cp->last_tagset = 0;
  cp->last_blk = NULL;

  /* allocate data blocks */
  cp->data = (byte_t *)calloc(nsets * assoc,
			      sizeof(struct cache_blk_t) +
			      (cp->balloc ? (bsize*sizeof(byte_t)) : 0));
  if (!cp->data)
    fatal("out of virtual memory");

  /* slice up the data blocks */
  for (bindex=0,i=0; i<nsets; i++)
    {
      cp->sets[i].way_head = NULL;
      cp->sets[i].way_tail = NULL;
      /* get a hash table, if needed */
      if (cp->hsize)
	{
	  cp->sets[i].hash =
	    (struct cache_blk_t **)calloc(cp->hsize,
					  sizeof(struct cache_blk_t *));
	  if (!cp->sets[i].hash)
	    fatal("out of virtual memory");
	}
      /* NOTE: all the blocks in a set *must* be allocated contiguously,
	 otherwise, block accesses through SET->BLKS will fail (used
	 during random replacement selection) */
      cp->sets[i].blks = CACHE_BINDEX(cp, cp->data, bindex);
      
      /* link the data blocks into ordered way chain and hash table bucket
         chains, if hash table exists */
      for (j=0; j<assoc; j++)
	{
	  /* locate next cache block */
	  blk = CACHE_BINDEX(cp, cp->data, bindex);
	  bindex++;

	  /* invalidate new cache block */
	  blk->status = 0;
	  blk->tag = 0;
	  blk->ready = 0;
	  blk->user_data = (usize != 0
			    ? (byte_t *)calloc(usize, sizeof(byte_t)) : NULL);

	  /* insert cache block into set hash table */
	  if (cp->hsize)
	    link_htab_ent(cp, &cp->sets[i], blk);

	  /* insert into head of way list, order is arbitrary at this point */
	  blk->way_next = cp->sets[i].way_head;
	  blk->way_prev = NULL;
	  if (cp->sets[i].way_head)
	    cp->sets[i].way_head->way_prev = blk;
	  cp->sets[i].way_head = blk;
	  if (!cp->sets[i].way_tail)
	    cp->sets[i].way_tail = blk;
        /*-AJS */
        cp->sets[i].way_data_tail = cp->sets[i].way_tag_tail = cp->sets[i].way_tail;
	}
    }
  return cp;
}

/* parse policy */
enum cache_policy			/* replacement policy enum */
cache_char2policy(char c)		/* replacement policy as a char */
{
  switch (c) {
  case 'l': return LRU;
  case 'r': return Random;
  case 'f': return FIFO;
  case 't': return TournamentLRU;
  default: fatal("bogus replacement policy, `%c'", c);
  }
}

/* print cache configuration */
void
cache_config(struct cache_t *cp,	/* cache instance */
	     FILE *stream)		/* output stream */
{
  fprintf(stream,
	  "cache: %s: %d sets, %d byte blocks, %d bytes user data/block\n",
	  cp->name, cp->nsets, cp->bsize, cp->usize);
  fprintf(stream,
	  "cache: %s: %d-way, `%s' replacement policy, write-back\n",
	  cp->name, cp->assoc,
	  cp->policy == LRU ? "LRU"
	  : cp->policy == Random ? "Random"
	  : cp->policy == FIFO ? "FIFO"
	  : (abort(), ""));
}

double Cache_Config_Ratio = 0;
/* register cache stats */
void
cache_reg_stats(struct cache_t *cp,	/* cache instance */
		struct stat_sdb_t *sdb)	/* stats database */
{
  char buf[512], buf1[512], *name;

  /* get a name for this cache */
  if (!cp->name || !cp->name[0])
    name = "<unknown>";
  else
    name = cp->name;
    
  /*Cache_Config_Ratio = (float)((Smaller_Cache_Access_Cnt+1)/(Larger_Cache_Access_Cnt + Smaller_Cache_Access_Cnt+1));
    printf("\n %d %D Value \n",  Cache_Config_Ratio);
  printf("\n %f Float Value\n",  Cache_Config_Ratio);
  printf("\n SCC: %d LCC: %d Value \n",  Smaller_Cache_Access_Cnt, Larger_Cache_Access_Cnt);
  printf("\n %2.5f Float Value\n",  (Smaller_Cache_Access_Cnt/Larger_Cache_Access_Cnt));*/

  /**************** -Jai*/
  /* -AJS added for tournament stats */
  if (TournamentLRU == cp->policy)
  {
  sprintf(buf, "%s.reconfigurations", name);
  stat_reg_counter(sdb, buf, "total number of reconfigurations", &cp->reconfigurations,
  0, NULL);
  sprintf(buf, "%s.tournaments", name);
  stat_reg_counter(sdb, buf, "total number of tournaments", &cp->tournaments, 0, NULL);
    sprintf(buf, "%s. Power-Saving ", name);
  stat_reg_CacheConfigRatio(sdb, buf, "Power Saving ", &Cache_Config_Ratio,
  0, NULL);
  
      sprintf(buf, "%s. Smaller Cache Cnt ", name);
  stat_reg_counter(sdb, buf, "SCC ", &Smaller_Cache_Access_Cnt,
  0, NULL);
      sprintf(buf, "%s. Largerer Cache Cnt ", name);
  stat_reg_counter(sdb, buf, "LCC ", &Larger_Cache_Access_Cnt,
  0, NULL);
  
  printf("\n %d %D Value \n",  Cache_Config_Ratio);
  printf("\n %12.5ld %LD Value\n",  Cache_Config_Ratio);
  
   //stat_reg_formula(sdb, "Cache_Config_Ratio",
	//	   "Cache_Run_Ratio",
	//	   "Smaller_Cache_Access_Cnt / Larger_Cache_Access_Cnt", /* format */NULL);
  
  
  }
  /**************** -Jai*/
  sprintf(buf, "%s.accesses", name);
  sprintf(buf1, "%s.hits + %s.misses", name, name);
  stat_reg_formula(sdb, buf, "total number of accesses", buf1, "%12.0f");
  sprintf(buf, "%s.hits", name);
  stat_reg_counter(sdb, buf, "total number of hits", &cp->hits, 0, NULL);
  sprintf(buf, "%s.misses", name);
  stat_reg_counter(sdb, buf, "total number of misses", &cp->misses, 0, NULL);
  sprintf(buf, "%s.mnm_misses", name);
  stat_reg_counter(sdb, buf, "total number MNM of misses", &cp->mnm_misses, 0, NULL);
  
  sprintf(buf, "%s.replacements", name);
  stat_reg_counter(sdb, buf, "total number of replacements",
		 &cp->replacements, 0, NULL);
  sprintf(buf, "%s.writebacks", name);
  stat_reg_counter(sdb, buf, "total number of writebacks",
		 &cp->writebacks, 0, NULL);
  sprintf(buf, "%s.invalidations", name);
  stat_reg_counter(sdb, buf, "total number of invalidations",
		 &cp->invalidations, 0, NULL);
  sprintf(buf, "%s.miss_rate", name);
  sprintf(buf1, "%s.misses / %s.accesses", name, name);
  stat_reg_formula(sdb, buf, "miss rate (i.e., misses/ref)", buf1, NULL);
  sprintf(buf, "%s.repl_rate", name);
  sprintf(buf1, "%s.replacements / %s.accesses", name, name);
  stat_reg_formula(sdb, buf, "replacement rate (i.e., repls/ref)", buf1, NULL);
  sprintf(buf, "%s.wb_rate", name);
  sprintf(buf1, "%s.writebacks / %s.accesses", name, name);
  stat_reg_formula(sdb, buf, "writeback rate (i.e., wrbks/ref)", buf1, NULL);
  sprintf(buf, "%s.inv_rate", name);
  sprintf(buf1, "%s.invalidations / %s.accesses", name, name);
  stat_reg_formula(sdb, buf, "invalidation rate (i.e., invs/ref)", buf1, NULL);
}

/* print cache stats */
void
cache_stats(struct cache_t *cp,		/* cache instance */
	    FILE *stream)		/* output stream */
{
  double sum = (double)(cp->hits + cp->misses);

  
  if (TournamentLRU == cp->policy)
    fprintf(stream,"cache: %s: %.0f hits %.0f misses %.0f repls %.0f invalidations %.0f tournaments %.0f reconfigurations\n",\
    cp->name, (double)cp->hits, (double)cp->misses, (double)cp->replacements,\
    (double)cp->invalidations, (double) cp->tournaments, (double)cp->reconfigurations);
  else
    fprintf(stream,"cache: %s: %.0f hits %.0f misses %.0f repls %.0f invalidations\n",\
      cp->name, (double)cp->hits, (double)cp->misses,\
      (double)cp->replacements, (double)cp->invalidations);
  fprintf(stream,"cache: %s: miss rate=%f  repl rate=%f  invalidation rate=%f\n", cp->name,\
    (double)cp->misses/sum, (double)(double)cp->replacements/sum, (double)cp->invalidations/sum);
}

//rv
int mnm_index(md_addr_t tag)
{
  int mask = 1 << MNM_INDEX_BITS;
  mask = mask -1;

  return tag & mask;

}
//rv
int mnm_check(struct cache_t *cp,md_addr_t addr)
{ int i=0;
  for(i=0;i<MNM_TABLE_SIZE;i++)
  {
    if(cp->mnm_table[i] == addr)
    {
      return FALSE;
    }
   
  }
  return TRUE;

}
//rv
int mnm_cursor = 0;
void mnm_update(struct cache_t *cp,md_addr_t addr,int type)
{
  int i=0;
  if(type == PLACE)
  {
    for(i=0;i<MNM_TABLE_SIZE;i++)
      {
        if(cp->mnm_table[i] == addr)
          {
            cp->mnm_table[i] = 0;
            return;
          }
      
      }
      
  }
  
  
  if(type == REPL)
  {
    cp->mnm_table[mnm_cursor] = addr; 
    mnm_cursor++;
    mnm_cursor = mnm_cursor % MNM_TABLE_SIZE;
    
    
  }
  return;
}


/* access a cache, perform a CMD operation on cache CP at address ADDR,
   places NBYTES of data at *P, returns latency of operation if initiated
   at NOW, places pointer to block user data in *UDATA, *P is untouched if
   cache blocks are not allocated (!CP->BALLOC), UDATA should be NULL if no
   user data is attached to blocks */
unsigned int				/* latency of access in cycles */
cache_access(struct cache_t *cp,	/* cache to access */
	     enum mem_cmd cmd,		/* access type, Read or Write */
	     md_addr_t addr,		/* address of access */
	     void *vp,			/* ptr to buffer for input/output */
	     int nbytes,		/* number of bytes to access */
	     tick_t now,		/* time of access */
	     byte_t **udata,		/* for return of user data ptr */
	     md_addr_t *repl_addr)	/* for address of replaced block */
{
  byte_t *p = vp;
  md_addr_t tag = CACHE_TAG(cp, addr);
  md_addr_t set = CACHE_SET(cp, addr);
  md_addr_t bofs = CACHE_BLK(cp, addr);
  struct cache_blk_t *blk, *repl;
  int lat = 0;


  /* default replacement address */
  if (repl_addr)
    *repl_addr = 0;

  /* check alignments */
  if ((nbytes & (nbytes-1)) != 0 || (addr & (nbytes-1)) != 0)
    fatal("cache: access error: bad size or alignment, addr 0x%08x", addr);

  /* access must fit in cache block */
  /* FIXME:
     ((addr + (nbytes - 1)) > ((addr & ~cp->blk_mask) + (cp->bsize - 1))) */
  if ((addr + nbytes) > ((addr & ~cp->blk_mask) + cp->bsize))
    fatal("cache: access error: access spans block, addr 0x%08x", addr);




  ///////////////////////////////
  ////* MNM table check *////////
  ///////////////////////////////
  
  // rv 
  int mcheck = TRUE;
  if(cp->isMNM== TRUE /*&& cmd != Write*/)//Jai
  {
      md_addr_t b_addr = CACHE_BADDR(cp,addr);
     mcheck = mnm_check(cp,b_addr);
     
      /*
     int i;
     
     printf("\n \n *** NEW ENTRY ***  \n mcheck is %d \n", mcheck);
     printf("\n mnm check BADDR = %x \n",b_addr);
     for(i=0;i<MNM_TABLE_SIZE;i++)
      printf("%x ", cp->mnm_table[i]);
      */

    if(mcheck == FALSE)
    {
    //Jai
      cp->mnm_misses++;
     // goto cache_miss;
     /* lat += cp->blk_access_fn(Read, CACHE_BADDR(cp, addr), cp->bsize,
			   repl, now+lat);
			
			return lat;//Jai*/
    }
    
  
  }

  //rv
    /* permissions are checked on cache misses */

  /* check for a fast hit: access to same block */
  if (CACHE_TAGSET(cp, addr) == cp->last_tagset)
    {
      /* hit in the same block */
      blk = cp->last_blk;
      goto cache_fast_hit;
    }
  
  if (cp->hsize)
    {
      /* higly-associativity cache, access through the per-set hash tables */
      int hindex = CACHE_HASH(cp, tag);

      for (blk=cp->sets[set].hash[hindex];
	   blk;
	   blk=blk->hash_next)
	{
	  if (blk->tag == tag && (blk->status & CACHE_BLK_VALID))
	    goto cache_hit;
	}
    }
  else
    {
      /* low-associativity cache, linear search the way list */
      for (blk=cp->sets[set].way_head;
	   blk;
	   blk=blk->way_next)
	{
	  if (blk->tag == tag && (blk->status & CACHE_BLK_VALID))
	    goto cache_hit;
	}
    }


  /* cache block not found */

  /* **MISS** */
  cache_miss:
    if(mcheck == FALSE)
        cp->true_mnm_misses++;    
  cp->misses++;

  /* select the appropriate block to replace, and re-link this entry to
     the appropriate place in the way list */
  switch (cp->policy) {
  case LRU:
  case FIFO:
    repl = cp->sets[set].way_tail;
    update_way_list(&cp->sets[set], repl, Head,cp);
    break;
  case Random:
    {
      int bindex = myrand() & (cp->assoc - 1);
      repl = CACHE_BINDEX(cp, cp->sets[set].blks, bindex);
    }
    break;
  default:
    panic("bogus replacement policy");
  }

  /* remove this block from the hash bucket chain, if hash exists */
  if (cp->hsize)
    unlink_htab_ent(cp, &cp->sets[set], repl);

  //rv
  if(cp->isMNM && repl->tag != 0)  //
  { // adddress of replaced block
    md_addr_t rpl_add = CACHE_MK_BADDR(cp, repl->tag, set);
    
    /*
    printf("\n REPLACE_BADD = %x ", rpl_add); 
    printf("\n REPLACE: repl->tag = %x ", repl->tag);
    printf("\n REPLACE: cp->tag_shift = %x ", cp->tag_shift);
    printf("\n REPLACE: set = %x ", set);
    printf("\n REPLACE: cp->set_shift = %x ", cp->set_shift);
   */
    
    mnm_update(cp,rpl_add,REPL);
   }

  /* blow away the last block to hit */
  cp->last_tagset = 0;
  cp->last_blk = NULL;

  /* write back replaced block data */
  if (repl->status & CACHE_BLK_VALID)
    {
      cp->replacements++;

      if (repl_addr)
	*repl_addr = CACHE_MK_BADDR(cp, repl->tag, set);

      /* don't replace the block until outstanding misses are satisfied */
      lat += BOUND_POS(repl->ready - now);
 
      /* stall until the bus to next level of memory is available */
      lat += BOUND_POS(cp->bus_free - (now + lat));
 
      /* track bus resource usage */
      cp->bus_free = MAX(cp->bus_free, (now + lat)) + 1;

      if (repl->status & CACHE_BLK_DIRTY)
	{
	  /* write back the cache block */
	  cp->writebacks++;
	  lat += cp->blk_access_fn(Write,
				   CACHE_MK_BADDR(cp, repl->tag, set),
				   cp->bsize, repl, now+lat);
		//printf("\n latency = %d \n ",lat);
	}
    }

  

  //rv
  
  if(cp->isMNM)
  {
    
    md_addr_t b_addr = CACHE_BADDR(cp,addr);
    mnm_update(cp,b_addr,PLACE);
    
    //printf("\n PLCD_ADDR = %x  PLCD_BADDR = %x ",addr, b_addr);
    
    
  }  
  /* update block tags */
  repl->tag = tag;
  repl->status = CACHE_BLK_VALID;	/* dirty bit set on update */
  
  
  /* read data block */
  /*lat += cp->blk_access_fn(Read, CACHE_BADDR(cp, addr), cp->bsize,
			   repl, now+lat);*/
			   
	int mnm_lat;		   
  if (mcheck == FALSE && cp->isMNM == TRUE) 
  {
    mnm_lat = MNM_LATENCY + cp->blk_access_fn(Read, CACHE_BADDR(cp, addr), cp->bsize,
			   repl, now + MNM_LATENCY) - cp->hit_latency;

	//	printf("\n %s MNM latency : %d \n",cp->name,mnm_lat);  
  } 
  
  else
  {
   lat += cp->blk_access_fn(Read, CACHE_BADDR(cp, addr), cp->bsize,
			   repl, now+lat);  
  //	printf("\n %s cache latency: %d \n",cp->name,lat);
	}
  
  /* copy data out of cache block */
  if (cp->balloc)
    {
      CACHE_BCOPY(cmd, repl, bofs, p, nbytes);
    }

  /* update dirty status */
  if (cmd == Write)
    repl->status |= CACHE_BLK_DIRTY;

  /* get user block data, if requested and it exists */
  if (udata)
    *udata = repl->user_data;

  /* update block status */
  repl->ready = now+lat;

  /* link this entry back into the hash table */
  if (cp->hsize)
    link_htab_ent(cp, &cp->sets[set], repl);

  /* return latency of the operation in case of MNM table*/
  //rv
  /*
  if(mcheck == FALSE && cp->isMNM == TRUE )
    return lat - cp->hit_latency + MNM_LATENCY;
  else
  */
  
  if (mcheck == FALSE && cp->isMNM == TRUE) 
  {
    //printf("\n returning %s MNM latency",cp->name);
    return mnm_lat;
    
  } 
  else
  {
   return lat;
	}
 


 cache_hit: /* slow hit handler */
  
  /* **HIT** */
  cp->hits++;

  /* copy data out of cache block, if block exists */
  if (cp->balloc)
    {
      CACHE_BCOPY(cmd, blk, bofs, p, nbytes);
    }

  /* update dirty status */
  if (cmd == Write)
    blk->status |= CACHE_BLK_DIRTY;

  /* if LRU replacement and this is not the first element of list, reorder */
  if (blk->way_prev && cp->policy == LRU)
    {
      /* move this block to head of the way (MRU) list */
      update_way_list(&cp->sets[set], blk, Head, cp);
    }

  /* tag is unchanged, so hash links (if they exist) are still valid */

  /* record the last block to hit */
  cp->last_tagset = CACHE_TAGSET(cp, addr);
  cp->last_blk = blk;

  /* get user block data, if requested and it exists */
  if (udata)
    *udata = blk->user_data;
  
  /* return first cycle data is available to access */
  int h_lat = (int) MAX(cp->hit_latency, (blk->ready - now));
  //printf("\n hit latency = %d d_lat = %d return lat = %d \n",cp->hit_latency, blk->ready - now, h_lat );
  return h_lat;

 cache_fast_hit: /* fast hit handler */
  
  /* **FAST HIT** */
  cp->hits++;

  /* copy data out of cache block, if block exists */
  if (cp->balloc)
    {
      CACHE_BCOPY(cmd, blk, bofs, p, nbytes);
    }

  /* update dirty status */
  if (cmd == Write)
    blk->status |= CACHE_BLK_DIRTY;

  /* this block hit last, no change in the way list */

  /* tag is unchanged, so hash links (if they exist) are still valid */

  /* get user block data, if requested and it exists */
  if (udata)
    *udata = blk->user_data;

  /* record the last block to hit */
  cp->last_tagset = CACHE_TAGSET(cp, addr);
  cp->last_blk = blk;

  /* return first cycle data is available to access */
  h_lat = (int) MAX(cp->hit_latency, (blk->ready - now));
  //printf("\n hit latency = %d d_lat = %d return lat = %d \n",cp->hit_latency, blk->ready - now, h_lat );
  return h_lat;
}



unsigned int				/* latency of access in cycles */
cache_l1_access(struct cache_t *cp,	/* cache to access */
	     enum mem_cmd cmd,		/* access type, Read or Write */
	     md_addr_t addr,		/* address of access */
	     void *vp,			/* ptr to buffer for input/output */
	     int nbytes,		/* number of bytes to access */
	     tick_t now,		/* time of access */
	     byte_t **udata,		/* for return of user data ptr */
	     md_addr_t *repl_addr)	/* for address of replaced block */
{
  byte_t *p = vp;
  md_addr_t tag = CACHE_TAG(cp, addr);
  md_addr_t set = CACHE_SET(cp, addr);
  md_addr_t bofs = CACHE_BLK(cp, addr);
  struct cache_blk_t *blk, *repl,*repl2;
  int lat = 0;
/*new tournament scheme*/
if(1)
{
  if(TournamentLRU == cp->policy)
  {
    //printf("\n Current Cahce Config : %d", cp->Current_Cache_Config);
    if(cp->Current_Cache_Config == T_SMALLER)
        Smaller_Cache_Access_Cnt++;
    else if(cp->Current_Cache_Config == T_LARGER)
        Larger_Cache_Access_Cnt++;
        
  Cache_Config_Ratio = ((Smaller_Cache_Access_Cnt+1)/(Larger_Cache_Access_Cnt +1));
  if (Smaller_Cache_Access_Cnt == 44485000) 
  {
    printf("\n SCC: %d LCC: %d Value \n",  Smaller_Cache_Access_Cnt, Larger_Cache_Access_Cnt);
    printf("\n %12.4f Cache_Config-Ratio ", Cache_Config_Ratio);
  }
  cp->accesses_since_tournament++;
  cp->tournament_accesses++;
  switch(cp->tournament_status)
  {
  case T_LARGER:
    if(cp->tournament_accesses > cp->TOURNAMENT_LENGTH
    || cp->TOURNAMENT_HITS_FOR_WIN < cp->tournament_hits )
    {
      if( cp->TOURNAMENT_HITS_FOR_WIN < cp->tournament_hits )
      {
        cache_reconfigure(cp,cp->tag_cols, cp->tag_cols);
        cp->reconfigurations++;
        cp->Current_Cache_Config = T_LARGER;
      }
      else
      {
        cache_reconfigure(cp,cp->data_cols, cp->data_cols);
        cp->Current_Cache_Config = T_SMALLER;
      }
      cp->accesses_since_tournament = 0;
      cp->tournament_status = T_NONE;
      fprintf(stderr,"larger tournament %u outcome, insn %u, accesses %u, hits %u, config %d -way\n",\
      (unsigned int)cp->tournaments, (unsigned int)sim_num_insn, cp->tournament_accesses, cp->tournament_hits, cp->data_cols);
    }
    break;
  case T_SMALLER:
    if(cp->tournament_accesses > cp->TOURNAMENT_LENGTH
    || cp->TOURNAMENT_HITS_FOR_WIN < cp->tournament_hits)
    {
    if( cp->TOURNAMENT_HITS_FOR_WIN >= cp->tournament_hits)
    {
    cache_reconfigure(cp,cp->data_cols - 1, cp->data_cols - 1);
    cp->reconfigurations++;
    cp->Current_Cache_Config = T_SMALLER;
    }
    cp->accesses_since_tournament = 0;
    cp->tournament_status = T_NONE;
    //fprintf(stderr,"smaller tournament %u outcome, insn %u, accesses %u, hits %u,\
    config %d -way\n",(unsigned int)cp->tournaments, (unsigned int)sim_num_insn, cp->tournament_accesses, cp->tournament_hits, cp->data_cols);
    }
    break;
  default:
  if(cp->miss_saturation > cp->MAX_MISS_SATURATION)
  {
    if(cp->data_cols < cp->assoc)
    {
      cp->tournaments++;
      cp->tournament_status = T_LARGER;
      cp->Current_Cache_Config = T_LARGER;
      cp->tournament_hits = cp->tournament_accesses = 0;
      cache_reconfigure(cp,cp->data_cols,cp->data_cols+1);
      fprintf(stderr,"larger tournament %u, insn %u, accesses_since_last %u,\
      miss_sat %d, miss_rat %f\n",
      (unsigned int)cp->tournaments,(unsigned int)sim_num_insn,cp->accesses_since_tournament,cp->miss_saturation
      ,((double)cp->misses)/((double)cp->hits+(double)cp->misses));
    }
  }
  else if (cp->accesses_since_tournament > cp->ACCESSES_BW_TOURNAMENTS)
  {
    if(cp->data_cols > 1)
    {
      cp->tournaments++;
      cp->tournament_status = T_SMALLER;
      cp->tournament_hits = cp->tournament_accesses = 0;
      fprintf(stderr,"smaller tournament %u, insn %u, accesses_since_last %u, miss_sat %d, miss_rat %f\n",\
        (unsigned int)cp->tournaments,(unsigned int)sim_num_insn,cp->accesses_since_tournament,cp->miss_saturation,((double)cp->misses)/((double)cp->hits+(double)cp->misses));
    }
  }
  break;
  }
  }
}
  /* default replacement address */
  if (repl_addr)
    *repl_addr = 0;

  /* check alignments */
  if ((nbytes & (nbytes-1)) != 0 || (addr & (nbytes-1)) != 0)
    fatal("cache: access error: bad size or alignment, addr 0x%08x", addr);

  /* access must fit in cache block */
  /* FIXME:
     ((addr + (nbytes - 1)) > ((addr & ~cp->blk_mask) + (cp->bsize - 1))) */
  if ((addr + nbytes) > ((addr & ~cp->blk_mask) + cp->bsize))
    fatal("cache: access error: access spans block, addr 0x%08x", addr);

  /* permissions are checked on cache misses */

  /* check for a fast hit: access to same block */
  if (CACHE_TAGSET(cp, addr) == cp->last_tagset)
    {
      /* hit in the same block */
      blk = cp->last_blk;
      goto cache_fast_hit;
    }
    
  if (TournamentLRU != cp->policy && cp->hsize) /*-AJS disabled hashing*/
    {
      /* higly-associativity cache, access through the per-set hash tables */
      int hindex = CACHE_HASH(cp, tag);

      for (blk=cp->sets[set].hash[hindex];
           blk;
           blk=blk->hash_next)
      {
        if (blk->tag == tag && (blk->status & CACHE_BLK_VALID))
          goto cache_hit;
      }
    }
  else
    {
      /* low-associativity cache, linear search the way list */
      if (TournamentLRU == cp->policy)
      {
        repl2 = cp->sets[set].way_tail;
        for (blk=cp->sets[set].way_head;
            blk && (blk->way_prev != cp->sets[set].way_data_tail);
            blk=blk->way_next)
        {
          if (blk->tag == tag && (blk->status & CACHE_BLK_VALID))
          {
            if(T_SMALLER == cp->tournament_status &&
                blk == cp->sets[set].way_data_tail)
                {
                  cp->tournament_hits++;
                }
            goto cache_hit;
          }
        }
        /* -AJS added for tournament */
        for(blk=cp->sets[set].way_data_tail;
            blk && (blk->way_prev != cp->sets[set].way_tag_tail);
            blk=blk->way_next)
        {
          if (blk->tag == tag )
          {
            repl2 = blk;/* repl2 will eventually move to head of way list.*/
            if(T_LARGER == cp->tournament_status)
              cp->tournament_hits++;
            goto cache_miss;
          }
        }
      }
      else
      {
        for (blk=cp->sets[set].way_head;
              blk;
              blk=blk->way_next)
        {
          if (blk->tag == tag && (blk->status & CACHE_BLK_VALID))
            goto cache_hit;
        }
      }
  }
cache_miss:/* -AJS added label*/
  /* cache block not found */

  /* **MISS** */
  cp->misses++;

  /* select the appropriate block to replace, and re-link this entry to
     the appropriate place in the way list */
  switch (cp->policy) {
  case LRU:
  case FIFO:
    repl = cp->sets[set].way_tail;
    update_way_list(&cp->sets[set], repl, Head, cp);
    break;
  case TournamentLRU:/* -AJS */
    cp->miss_saturation++;
    repl = cp->sets[set].way_data_tail;
    update_way_list(&cp->sets[set], repl2, Head,cp);
    break;
  case Random:
    {
      int bindex = myrand() & (cp->assoc - 1);
      repl = CACHE_BINDEX(cp, cp->sets[set].blks, bindex);
    }
    break;
  default:
    panic("bogus replacement policy");
  }

  /* remove this block from the hash bucket chain, if hash exists */
  if (cp->hsize)
    unlink_htab_ent(cp, &cp->sets[set], repl);

  /* blow away the last block to hit */
  cp->last_tagset = 0;
  cp->last_blk = NULL;

  /* write back replaced block data */
  if (repl->status & CACHE_BLK_VALID)
    {
      cp->replacements++;

      if (repl_addr)
	*repl_addr = CACHE_MK_BADDR(cp, repl->tag, set);
 
      /* don't replace the block until outstanding misses are satisfied */
      lat += BOUND_POS(repl->ready - now);
 
      /* stall until the bus to next level of memory is available */
      lat += BOUND_POS(cp->bus_free - (now + lat));
 
      /* track bus resource usage */
      cp->bus_free = MAX(cp->bus_free, (now + lat)) + 1;

      if (repl->status & CACHE_BLK_DIRTY)
	{
	  /* write back the cache block */
	  cp->writebacks++;
	  lat += cp->blk_access_fn(Write,
				   CACHE_MK_BADDR(cp, repl->tag, set),
				   cp->bsize, repl, now+lat);
	}
    }
  /*-AJS Overwrite the new head.*/
  if (TournamentLRU == cp->policy)
  repl = repl2;
  /* update block tags */
  repl->tag = tag;
  repl->status = CACHE_BLK_VALID;	/* dirty bit set on update */

  /* read data block */
  lat += cp->blk_access_fn(Read, CACHE_BADDR(cp, addr), cp->bsize,
			   repl, now+lat);

  /* copy data out of cache block */
  if (cp->balloc)
    {
      CACHE_BCOPY(cmd, repl, bofs, p, nbytes);
    }

  /* update dirty status */
  if (cmd == Write)
    repl->status |= CACHE_BLK_DIRTY;

  /* get user block data, if requested and it exists */
  if (udata)
    *udata = repl->user_data;

  /* update block status */
  repl->ready = now+lat;

  /* link this entry back into the hash table */
  if (cp->hsize)
    link_htab_ent(cp, &cp->sets[set], repl);

  /* return latency of the operation */
  return lat;


 cache_hit: /* slow hit handler */
  
  /* **HIT** */
  cp->hits++;
  /*-AJS Added for TournamentLRU*/
  cp->miss_saturation--;
  if (cp->miss_saturation < 0)
  cp->miss_saturation=0;
  /* copy data out of cache block, if block exists */
  if (cp->balloc)
    {
      CACHE_BCOPY(cmd, blk, bofs, p, nbytes);
    }

  /* update dirty status */
  if (cmd == Write)
    blk->status |= CACHE_BLK_DIRTY;

  /* if LRU replacement and this is not the first element of list, reorder */
  if (blk->way_prev && (cp->policy == LRU || cp->policy == TournamentLRU))
    {
      /* move this block to head of the way (MRU) list */
      update_way_list(&cp->sets[set], blk, Head, cp);
    }

  /* tag is unchanged, so hash links (if they exist) are still valid */

  /* record the last block to hit */
  cp->last_tagset = CACHE_TAGSET(cp, addr);
  cp->last_blk = blk;

  /* get user block data, if requested and it exists */
  if (udata)
    *udata = blk->user_data;

  /* return first cycle data is available to access */
  return (int) MAX(cp->hit_latency, (blk->ready - now));

 cache_fast_hit: /* fast hit handler */
  
  /* **FAST HIT** */
  cp->hits++;
  /*-AJS Added for TournamentLRU*/
  cp->miss_saturation--;
  if (cp->miss_saturation<0)
  cp->miss_saturation=0;
  /* copy data out of cache block, if block exists */
  if (cp->balloc)
    {
      CACHE_BCOPY(cmd, blk, bofs, p, nbytes);
    }

  /* update dirty status */
  if (cmd == Write)
    blk->status |= CACHE_BLK_DIRTY;

  /* this block hit last, no change in the way list */

  /* tag is unchanged, so hash links (if they exist) are still valid */

  /* get user block data, if requested and it exists */
  if (udata)
    *udata = blk->user_data;

  /* record the last block to hit */
  cp->last_tagset = CACHE_TAGSET(cp, addr);
  cp->last_blk = blk;

  /* return first cycle data is available to access */
  return (int) MAX(cp->hit_latency, (blk->ready - now));
}



/* -AJS reconfigure the cache by powering down particular columns */
void
cache_reconfigure(struct cache_t *cp,/*cache instance to change */
  int data_columns, /*number of columns to enable for data */
  int tag_columns) /*number of columns to allow tag lookups */
  {
    int set_count = 0;
    if(cp->assoc < data_columns)
      fatal("Cache only has '%d' columns. '%d' is not a valid number of data columns for this cache",cp->assoc,data_columns);
    if(cp->assoc < tag_columns)
      fatal("Cache only has '%d' columns. '%d' is not a valid number of tag columns for this cache",cp->assoc,tag_columns);
    if(data_columns > tag_columns)
      fatal("The number of tag columns must be greater than or equal to the number of data columns");
    cp->data_cols = data_columns;
    cp->tag_cols = tag_columns;
    /* reset the way_data_tail and way_tag_tail for the entire cache */
    for(set_count=0;set_count<cp->nsets;set_count++)
    {
      cache_reconfigure_set(cp,&cp->sets[set_count]);
    }
  }
void
cache_reconfigure_set(struct cache_t *cp,struct cache_set_t *set)
{
  int i = 0;
  set->way_tag_tail = set->way_tail;
  for(i = cp->assoc; i > cp->tag_cols;i--)
    set->way_tag_tail = set->way_tag_tail->way_prev;
  
  set->way_data_tail = set->way_tag_tail;
  for(i=cp->tag_cols; i > cp->data_cols;i--)
    set->way_data_tail = set->way_data_tail->way_prev;
}

/* return non-zero if block containing address ADDR is contained in cache
   CP, this interface is used primarily for debugging and asserting cache
   invariants */
int					/* non-zero if access would hit */
cache_probe(struct cache_t *cp,		/* cache instance to probe */
	    md_addr_t addr)		/* address of block to probe */
{
  md_addr_t tag = CACHE_TAG(cp, addr);
  md_addr_t set = CACHE_SET(cp, addr);
  struct cache_blk_t *blk;

  /* permissions are checked on cache misses */

  if (cp->hsize)
  {
    /* higly-associativity cache, access through the per-set hash tables */
    int hindex = CACHE_HASH(cp, tag);
    
    for (blk=cp->sets[set].hash[hindex];
	 blk;
	 blk=blk->hash_next)
    {	
      if (blk->tag == tag && (blk->status & CACHE_BLK_VALID))
	  return TRUE;
    }
  }
  else
  {
    /* low-associativity cache, linear search the way list */
    for (blk=cp->sets[set].way_head;
	 blk;
	 blk=blk->way_next)
    {
      if (blk->tag == tag && (blk->status & CACHE_BLK_VALID))
	  return TRUE;
    }
  }
  
  /* cache block not found */
  return FALSE;
}

/* flush the entire cache, returns latency of the operation */
unsigned int				/* latency of the flush operation */
cache_flush(struct cache_t *cp,		/* cache instance to flush */
	    tick_t now)			/* time of cache flush */
{
  int i, lat = cp->hit_latency; /* min latency to probe cache */
  struct cache_blk_t *blk;

  /* blow away the last block to hit */
  cp->last_tagset = 0;
  cp->last_blk = NULL;

  //MNM table init
  //rv
  if(cp->isMNM)
    mnm_init(cp);

  /* no way list updates required because all blocks are being invalidated */
  for (i=0; i<cp->nsets; i++)
    {
      for (blk=cp->sets[i].way_head; blk; blk=blk->way_next)
	{
	  if (blk->status & CACHE_BLK_VALID)
	    {
	      cp->invalidations++;
	      blk->status &= ~CACHE_BLK_VALID;

	      if (blk->status & CACHE_BLK_DIRTY)
		{
		  /* write back the invalidated block */
          	  cp->writebacks++;
		  lat += cp->blk_access_fn(Write,
					   CACHE_MK_BADDR(cp, blk->tag, i),
					   cp->bsize, blk, now+lat);
		}
	    }
	}
    }

  /* return latency of the flush operation */
  return lat;
}

/* flush the block containing ADDR from the cache CP, returns the latency of
   the block flush operation */
unsigned int				/* latency of flush operation */
cache_flush_addr(struct cache_t *cp,	/* cache instance to flush */
		 md_addr_t addr,	/* address of block to flush */
		 tick_t now)		/* time of cache flush */
{
  md_addr_t tag = CACHE_TAG(cp, addr);
  md_addr_t set = CACHE_SET(cp, addr);
  struct cache_blk_t *blk;
  int lat = cp->hit_latency; /* min latency to probe cache */
  //rv
  //MNM tag entry decrement
  if(cp->isMNM)
    mnm_update(cp,tag,DECR);

  if (cp->hsize)
    {
      /* higly-associativity cache, access through the per-set hash tables */
      int hindex = CACHE_HASH(cp, tag);

      for (blk=cp->sets[set].hash[hindex];
	   blk;
	   blk=blk->hash_next)
	{
	  if (blk->tag == tag && (blk->status & CACHE_BLK_VALID))
	    break;
	}
    }
  else
    {
      /* low-associativity cache, linear search the way list */
      for (blk=cp->sets[set].way_head;
	   blk;
	   blk=blk->way_next)
	{
	  if (blk->tag == tag && (blk->status & CACHE_BLK_VALID))
	    break;
	}
    }

  if (blk)
    {
      cp->invalidations++;
      blk->status &= ~CACHE_BLK_VALID;

      /* blow away the last block to hit */
      cp->last_tagset = 0;
      cp->last_blk = NULL;

      if (blk->status & CACHE_BLK_DIRTY)
	{
	  /* write back the invalidated block */
          cp->writebacks++;
	  lat += cp->blk_access_fn(Write,
				   CACHE_MK_BADDR(cp, blk->tag, set),
				   cp->bsize, blk, now+lat);
	}
      /* move this block to tail of the way (LRU) list */
      update_way_list(&cp->sets[set], blk, Tail, cp);
    }

  /* return latency of the operation */
  return lat;
}