/**
 *	(C) Copyright Boitho 2007-2008. Written by Magnus Gal�en
 *
 *	In this first version, we ignore synonyms of several words length.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../ds/dcontainer.h"
#include "../ds/dvector.h"
#include "../ds/dset.h"
#include "query_parser.h"
#include "read_thesaurus.h"


int main( int argc, char *argv[] )
{
    if (argc!=4)
	{
	    printf( "Usage: %s <stemwords> <thesaurus> <query>\n\n", argv[0] );
	    return -1;
	}

    int		paramnr;
    container	*W[2];

    printf("loading..."); fflush(stdout);

    for (paramnr=1; paramnr<=2; paramnr++)
	{
	    FILE	*file = fopen( argv[paramnr], "r" );

	    if (!file)
		{
		    fprintf( stderr, "Could not open %s.\n", argv[paramnr] );
		    return -1;
		}

	    // Get filesize:
	    struct stat	fileinfo;
	    fstat( fileno( file ), &fileinfo );

	    int		size = fileinfo.st_size;
	    char	*buf = (char*)malloc(sizeof(char)*size);

	    int	i;
	    for (i=0; i<size;)
		{
		    i+= fread( (void*)&(buf[i]), sizeof(char), size-i, file );
		}

	    W[paramnr-1] = read_thesaurus(buf, size);

	    free(buf);
	    fclose(file);
	}

    printf("done\n");

/*****/
    int			x,i,j,k;
    query_array		qa;

    get_query( argv[3], strlen(argv[3]), &qa);

    for (x=0; x<qa.n; x++)
	{
	    if (qa.query[x].operand == QUERY_WORD)
		{
		    container		*Q_1 = vector_container( string_container() );

		    vector_pushback(Q_1, qa.query[x].s[0]);
		    printf("%s", qa.query[x].s[0]);

		    container		*alt = set_container( string_container() );
//		    printf("Inserting[a] %s... ", qa.query[x].s[0]);
		    set_insert(alt, qa.query[x].s[0]);

		    container		*layer1_stems = get_synonyms(W[0], Q_1);
		    vector_pushback(layer1_stems);
		    vector_pushback(vector_get(layer1_stems,vector_size(layer1_stems)-1).C, qa.query[x].s[0]);

		    for (i=0; i<vector_size(layer1_stems); i++)
			{
			    container	*R_1 = vector_get(layer1_stems,i).C;

			    // Only do singleworded stems and synonyms for now:
			    if (vector_size(R_1) == 1)
				{
//				    printf("Inserting[b] %s... ", (char*)vector_get(R_1,0).ptr);
				    set_insert(alt, (char*)vector_get(R_1,0).ptr);

				    container	*Q_2 = vector_container( string_container() );
				    vector_pushback(Q_2, (char*)vector_get(R_1,0).ptr);
				    container	*layer2_syn = get_synonyms(W[1], Q_2);
				    vector_pushback(layer2_syn);
				    vector_pushback(vector_get(layer2_syn,vector_size(layer2_syn)-1).C, (char*)vector_get(R_1,0).ptr);

				    for (j=0; j<vector_size(layer2_syn); j++)
					{
					    container	*R_2 = vector_get(layer2_syn,j).C;
					    if (vector_size(R_2) == 1)
						{
//						    printf("Inserting[c] %s... ",  (char*)vector_get(R_2,0).ptr);
						    set_insert(alt, (char*)vector_get(R_2,0).ptr);

						    container	*Q_3 = vector_container( string_container() );
						    vector_pushback(Q_3, (char*)vector_get(R_2,0).ptr);
						    container	*layer3_stems = get_synonyms(W[0], Q_3);
						    vector_pushback(layer3_stems);
						    vector_pushback(vector_get(layer3_stems,vector_size(layer3_stems)-1).C, (char*)vector_get(R_2,0).ptr);

						    for (k=0; k<vector_size(layer3_stems); k++)
						        {
						    	    container	*R_3 = vector_get(layer3_stems,k).C;
							    if (vector_size(R_3) == 1)
							        {
//								    printf("Inserting[d] %s... ", (char*)vector_get(R_3,0).ptr);
								    set_insert(alt, (char*)vector_get(R_3,0).ptr);
								}
							}

						    destroy_synonyms(layer3_stems);
						    destroy(Q_3);
						}
					}

				    destroy_synonyms(layer2_syn);
				    destroy(Q_2);
			        }
			}

		    set_remove(alt, qa.query[x].s[0]);

		    iterator	sit = set_begin(alt);
		    for (; sit.valid; sit=set_next(sit))
			{
			    printf(" | %s", (char*)set_key(sit).ptr);
			}

		    printf("\n");

		    destroy_synonyms(layer1_stems);
		    destroy(alt);
		    destroy(Q_1);
		}
/*
	    printf("(%c)", qa.query[i].operand );

		for (j=0; j<qa.query[i].n; j++)
		{
			printf(" %s", qa.query[i].s[j]);
		}

	    printf("\n");
*/
	}

    printf("finished\n");

    destroy_query( &qa );

    destroy(W[0]);
    destroy(W[1]);

/*****/

/*
    container	*text = vector_container( string_container() );
    vector_pushback(text, argv[2]);

    int		i, j;
    container	*V = get_synonyms(T, text);

    printf("Synonyms:");
    if (V!=NULL)
	{
	    for (i=0; i<vector_size(V); i++)
		{
		    container	*W = vector_get(V,i).C;
		    for (j=0; j<vector_size(W); j++)
			printf(" %s", (char*)vector_get(W,j).ptr);

		    if (i<(vector_size(V)-1)) printf(",");
		}
	    printf("\n");
	}
    else printf(" <none>\n");

    destroy(text);
*/
//    destroy_synonyms(V);
//    destroy(T);

    return 0;
}