// System libraries
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <zlib.h>

// Include my libraries
#include "Array.h"
#include "Hashmap.h"

// Initialize the kseq library
#include "kseq.h"
KSEQ_INIT(gzFile, gzread)

// Print usage statement
void print_usage(FILE *outstream, char *progname)
{
  fprintf( outstream, "\nUsage: %s [options] genomeseq \"rvdseq\"\n"
           "  Options:\n"
           "    -f|--forwardonly      only search the forward strand of the genomic sequence\n"
           "    -h|--help             print this help message and exit\n"
           "    -o|--outfile          file to which output will be written; default is terminal\n"
           "                          (STDOUT)\n"
           "    -w|--weight           user-defined weight; default it 0.9\n"
           "    -x|--cutoffmult       multiple of best score at which potential sites will be\n"
           "                          filtered\n\n", progname );
}

// Identify and print out TAL effector binding sites
void find_binding_sites(kseq_t *seq, Array *rvdseq, Hashmap *diresidue_scores, double cutoff, int forwardonly, FILE *outstream)
{
  int i, j;

  if(array_size(rvdseq) > seq->seq.l)
  {
    fprintf(stderr, "Warning: skipping sequence '%s' since it is shorter than the RVD sequence\n", seq->seq.s);
    return;
  }

  for(i = 1; i <= seq->seq.l - array_size(rvdseq); i++)
  {
    if(seq->seq.s[i-1] == 'T' || seq->seq.s[i-1] == 't')
    {
      double cumscore = 0.0;
      for(j = 0; j < array_size(rvdseq); j++)
      {
        char *rvd = array_get(rvdseq, j);
        double *scores = hashmap_get(diresidue_scores, rvd);
        switch(seq->seq.s[i+j])
        {
          case 'A':
            cumscore += scores[0];
            break;
          case 'a':
            cumscore += scores[0];
            break;
          case 'C':
            cumscore += scores[1];
            break;
          case 'c':
            cumscore += scores[1];
            break;
          case 'G':
            cumscore += scores[2];
            break;
          case 'g':
            cumscore += scores[2];
            break;
          case 'T':
            cumscore += scores[3];
            break;
          case 't':
            cumscore += scores[3];
            break;
        }

        if(cumscore > cutoff)
          break;
      }

      if(cumscore <= cutoff)
      {
        fprintf(outstream, "%s:%u\n", seq->name.s, i);
      }
    }
  }
}

// Allocate memory for an array of 4 doubles
double *double_array(double a, double c, double g, double t)
{
  double *array = malloc(sizeof(double)*4);
  array[0] = a;
  array[1] = c;
  array[2] = g;
  array[3] = t;
  return array;
}

// Allocate memory for an array of 4 integers
int *int_array(int a, int c, int g, int t)
{
  int *array = malloc(sizeof(int)*4);
  array[0] = a;
  array[1] = c;
  array[2] = g;
  array[3] = t;
  return array;
}

// Compute diresidue probabilities from observed counts
Hashmap *get_diresidue_probabilities(double w)
{
  char **diresidues;
  Hashmap *diresidue_counts, *diresidue_probabilities;
  int i, num_diresidues;

  // First, use a hashmap to store counts
  diresidue_counts = hashmap_new(64);

  // Add known counts
  hashmap_add(diresidue_counts, "HD", int_array( 7, 99,  0,  1));
  hashmap_add(diresidue_counts, "NI", int_array(58,  6,  0,  0));
  hashmap_add(diresidue_counts, "NG", int_array( 6,  6,  1, 57));
  hashmap_add(diresidue_counts, "NN", int_array(21,  8, 26,  2));
  hashmap_add(diresidue_counts, "NS", int_array(20,  6,  4,  0));
  hashmap_add(diresidue_counts, "N*", int_array( 1, 11,  1,  7));
  hashmap_add(diresidue_counts, "HG", int_array( 1,  2,  0, 15));
  hashmap_add(diresidue_counts, "HA", int_array( 1,  4,  1,  0));
  hashmap_add(diresidue_counts, "ND", int_array( 0,  4,  0,  0));
  hashmap_add(diresidue_counts, "NK", int_array( 0,  0,  2,  0));
  hashmap_add(diresidue_counts, "HI", int_array( 0,  1,  0,  0));
  hashmap_add(diresidue_counts, "HN", int_array( 0,  0,  1,  0));
  hashmap_add(diresidue_counts, "NA", int_array( 0,  0,  1,  0));
  hashmap_add(diresidue_counts, "IG", int_array( 0,  0,  0,  1));
  hashmap_add(diresidue_counts, "H*", int_array( 0,  0,  0,  1));

  // Add unknown counts
  hashmap_add(diresidue_counts, "S*", int_array(1, 1, 1, 1));
  hashmap_add(diresidue_counts, "NH", int_array(1, 1, 1, 1));
  hashmap_add(diresidue_counts, "YG", int_array(1, 1, 1, 1));
  hashmap_add(diresidue_counts, "SN", int_array(1, 1, 1, 1));
  hashmap_add(diresidue_counts, "SS", int_array(1, 1, 1, 1));
  hashmap_add(diresidue_counts, "NC", int_array(1, 1, 1, 1));
  hashmap_add(diresidue_counts, "HH", int_array(1, 1, 1, 1));

  diresidue_probabilities = hashmap_new(64);
  diresidues = hashmap_keys(diresidue_counts);
  num_diresidues = hashmap_size(diresidue_counts);
  for(i = 0; i < num_diresidues; i++)
  {
    double p[4], psum;
    int j;
    int *counts = hashmap_get(diresidue_counts, diresidues[i]);
    int sum = counts[0] + counts[1] + counts[2] + counts[3];

    psum = 0.0;
    for(j = 0; j < 4; j++)
    {
      p[j] = (double)counts[j] / (double)sum * w * 10.0 + (1.0 - w) / 4.0 * 10.0;
      psum += p[j];
    }

    // Normalize
    for(j = 0; j < 4; j++)
      p[j] /= psum;

    hashmap_add(diresidue_probabilities, diresidues[i], double_array(p[0], p[1], p[2], p[3]));
  }

  free(diresidues);
  hashmap_delete(diresidue_counts, free);
  return diresidue_probabilities;
}

// Convert diresidue probabilities into scores
Hashmap *convert_probabilities_to_scores(Hashmap *diresidue_probabilities)
{
  char **diresidues;
  int i, j, num_diresidues;
  Hashmap *diresidue_scores = hashmap_new(64);

  diresidues = hashmap_keys(diresidue_probabilities);
  num_diresidues = hashmap_size(diresidue_probabilities);
  for(i = 0; i < num_diresidues; i++)
  {
    double *probs = hashmap_get(diresidue_probabilities, diresidues[i]);
    for(j = 0; j < 4; j++)
      probs[j] = -1.0 * log(probs[j]);

    hashmap_add(diresidue_scores, diresidues[i], probs);
  }

  free(diresidues);
  return diresidue_scores;
}

// Given a sequence of RVDs, calculate the optimal score
double get_best_score(Array *rvdseq, Hashmap *rvdscores)
{
  double best_score = 0.0;
  int i, j;

  for(i = 0; i < array_size(rvdseq); i++)
  {
    char *rvd = array_get(rvdseq, i);
    double *scores = hashmap_get(rvdscores, rvd);
    double min_score = -1.0;
    if(scores == NULL)
      return -1.0;

    /*
    fprintf(stderr, "scores = [");
    for(j = 0; j < 4; j++)
    {
      if(j > 0)
        fprintf(stderr, ", ");
      fprintf(stderr, "%.4lf", scores[j]);
    }
    fprintf(stderr, "], ");
    */

    for(j = 0; j < 4; j++)
    {
      if(j == 0 || scores[j] < min_score)
        min_score = scores[j];
    }
    //fprintf(stderr, "min score %d (%s): %.4lf\n", i, rvd, min_score);
    best_score += min_score;
  }

  return best_score;
}

// Main method
int main(int argc, char **argv)
{
  // Program arguments and options
  char *progname, *seqfilename, *rvdstring;
  int forwardonly;
  FILE *outstream;
  double w, x;

  // Program variable domain
  Array *rvdseq;
  char *tok;
  gzFile seqfile;
  kseq_t *seq;
  int i;

  // Set option defaults
  forwardonly = 0;
  outstream = stdout;
  w = 0.9;
  x = 3.0;

  // Parse options
  progname = argv[0];
  int opt, optindex;
  const char *optstr = "fho:w:x:";
  const struct option otsf_options[] =
  {
    { "forwardonly", no_argument, NULL, 'f' },
    { "help", no_argument, NULL, 'h' },
    { "outfile", required_argument, NULL, 'o' },
    { "weight", required_argument, NULL, 'w' },
    { "cutoffmult", required_argument, NULL, 'x' },
    { NULL, no_argument, NULL, 0 },
  };

  for( opt = getopt_long(argc, argv + 0, optstr, otsf_options, &optindex);
       opt != -1;
       opt = getopt_long(argc, argv + 0, optstr, otsf_options, &optindex) )
  {
    switch(opt)
    {
      case 'f':
        forwardonly = 1;
        break;

      case 'h':
        print_usage(stdout, progname);
        return 0;

      case 'o':
        outstream = fopen(optarg, "w");
        if(!outstream)
        {
          fprintf(stderr, "Error: unable to open output file '%s'\n", optarg);
          return 1;
        }
        break;

       case 'w':
         if( sscanf(optarg, "%lf", &w) == EOF )
         {
           fprintf(stderr, "Error: unable to convert weight '%s' to a double\n", optarg);
           return 1;
         }
         break;

       case 'x':
         if( sscanf(optarg, "%lf", &x) == EOF )
         {
           fprintf(stderr, "Error: unable to convert cutoff multiple '%s' to a double\n", optarg);
           return 1;
         }
         break;
    }
  }

  // Parse arguments
  if(argc - optind != 2)
  {
    fputs("Error: must provide genomic sequence (file) and RVD sequence (string)...no more, no less\n", stderr);
    print_usage(stderr, progname);
    return 1;
  }
  seqfilename = argv[optind];
  rvdstring = argv[optind + 1];
  fprintf(stderr, "Using: seqfile='%s', RDVseq='%s'\n", seqfilename, rvdstring);

  rvdseq = array_new( sizeof(char *) );
  tok = strtok(rvdstring, " ");
  while(tok != NULL)
  {
    char *r = strdup(tok);
    array_add(rvdseq, r);
    tok = strtok(NULL, " ");
  }
  fprintf(stderr, "RVD seq:");
  for(i = 0; i < array_size(rvdseq); i++)
  {
    char *temp = (char *)array_get(rvdseq, i);
    fprintf(stderr, " '%s'", temp);
  }
  fprintf(stderr, "\n");

  // Get RVD/bp matching scores
  Hashmap *diresidue_probabilities = get_diresidue_probabilities(w);
  Hashmap *diresidue_scores = convert_probabilities_to_scores(diresidue_probabilities);
  hashmap_delete(diresidue_probabilities, NULL);

  // Compute optimal score for this RVD sequence
  double best_score = get_best_score(rvdseq, diresidue_scores);
  fprintf(stderr, "Best score: %.4lf\n", best_score);

  // Open genomic sequence file
  seqfile = gzopen(seqfilename, "r");
  if(!seqfile)
  {
    fprintf(stderr, "Error: unable to open genomic sequence '%s'\n", seqfilename);
    return 1;
  }

  // Process
  seq = kseq_init(seqfile);
  while((i = kseq_read(seq)) >= 0)
  {
    printf("Found seq '%s', length %ld\n", seq->name.s, seq->seq.l);
    find_binding_sites(seq, rvdseq, diresidue_scores, best_score * x, forwardonly, outstream);
  }
  kseq_destroy(seq);
  gzclose(seqfile);

  // Free memory
  for(i = 0; i < array_size(rvdseq); i++)
  {
    char *temp = (char *)array_get(rvdseq, i);
    free(temp);
  }
  array_delete(rvdseq);
  hashmap_delete(diresidue_scores, free);

  return 0;
}
