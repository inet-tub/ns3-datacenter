#ifndef CDF_H
#define CDF_H

#include <stdio.h>
#include <stdlib.h>

#define TG_CDF_TABLE_ENTRY 32

struct cdf_entry
{
    double value;
    double cdf;
};

/* CDF distribution */
struct cdf_table
{
    struct cdf_entry *entries;
    int num_entry;  /* number of entries in CDF table */
    int max_entry;  /* maximum number of entries in CDF table */
    double min_cdf; /* minimum value of CDF (default 0) */
    double max_cdf; /* maximum value of CDF (default 1) */
};

/* initialize a CDF distribution */
void init_cdf(struct cdf_table *table);

/* free resources of a CDF distribution */
void free_cdf(struct cdf_table *table);

/* get CDF distribution from a given file */
void load_cdf(struct cdf_table *table, const char *file_name);

/* print CDF distribution information */
void print_cdf(struct cdf_table *table);

/* get average value of CDF distribution */
double avg_cdf(struct cdf_table *table);

/* Generate a random value based on CDF distribution */
double gen_random_cdf(struct cdf_table *table);

double var_cdf(struct cdf_table *table);

#endif
