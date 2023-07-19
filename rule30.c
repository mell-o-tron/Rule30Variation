#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>

#define loop(index, s, n) for (int index = s; index < n; index++)
#define PRECISION 100000000

typedef struct {
    unsigned char * old_line;
    unsigned char * new_line;
    int size;

    int start;
    int end;

} thread_args;

/* evaluates a cell state based on three cells */
int compute_cell (unsigned char a, unsigned char b, unsigned char c){
    return (a && !b && !c) || (!a && b) || (!a && c);
}

/* builds part of the new line */
void *rule30_thread (void *vargp)
{

    thread_args *args = (thread_args*) vargp;

//     printf("start = %d\t end = %d\n", args -> start, args -> end);

    loop(i, args -> start, args -> end + 1){
        args -> new_line[i] = compute_cell(args -> old_line[i-1], args -> old_line[i], args -> old_line[i+1]);
    }

    return NULL;
}




/* prints one line of rule 30 */
int print_line(unsigned char* line, int size){
    loop (i, 0, size) putc (line[i] == 0 ? '-' : '%', stdout);
    puts("");
}

/* writes the next line in new_line */
int get_next_line (unsigned char* old_line, unsigned char* new_line, int size, thread_args** t_args, int chunk_size, int n_chunks){
    pthread_t * threads = malloc(sizeof(pthread_t) * n_chunks);

    loop(i, 0, n_chunks){
        pthread_t x;
        threads[i] = x;
    }

    loop(i, 0, n_chunks){

        t_args[i] -> old_line = old_line;
        t_args[i] -> new_line = new_line;
        t_args[i] -> size = size;
        t_args[i] -> start = 1 + i * chunk_size;

        if(i == chunk_size - 1)
            t_args[i] -> end = size - 2;

        t_args[i] -> end   = (i + 1) * chunk_size;
    }

    loop(i, 0, n_chunks){
        pthread_create(&threads[i], NULL, rule30_thread, t_args[i]);
    }

    loop(i, 0, n_chunks){
        pthread_join(threads[i], NULL);
    }


    free(threads);

    new_line[0] = compute_cell (1, old_line[0], old_line[1]);
    new_line[size - 1] = compute_cell (old_line[size - 2], old_line[size - 1], size % 2);
}

/* makes the new line old */
int birthday (unsigned char* old_line, unsigned char* new_line, int size){
    loop (i, 0, size) old_line[i] = new_line[i];
}

/* computes the ratio between the messy
 * bit and the pattern on the left      */
long int compute_ratio (unsigned char* line, int size){
    long int preserved_left = 0;
    long int preserved_right = 0;

    loop(i, 0, size){
        if (line[i] != i % 2){
            preserved_left += i;
            break;
        }
    }

    loop(i, 0, size){
        int j = size - i;

        int line_j = line[j] % 129;
        if (line_j != j % 2){
            preserved_right += i;
            break;
        }
    }

    long int messy_bit = size - (preserved_left + preserved_right);

    return (messy_bit * PRECISION) / (preserved_left);
}

int main(){
    int size = 1600000;

    // creates output file
    FILE *output;
    output = fopen("output.txt", "w");
    fclose(output);

    assert(size > 1);

    unsigned char* old_line = malloc (size);
    unsigned char* new_line = malloc (size);

    loop(i, 0, size) old_line[i] = i % 2;

    old_line[0] = 1;

    int chunk_size = 200000;
    int n_chunks = size / chunk_size;


    thread_args** t_args = malloc(sizeof(thread_args*) * n_chunks);

    loop(i, 0, n_chunks){
        t_args [i] = malloc (sizeof (thread_args));
    }

    long int prev_time = time(0);



    puts("beginning...");
    loop (i, 1, 1000000){
//         print_line(old_line, size);
//         printf("%f\n", compute_ratio(old_line, size));

        if(i % 1000 == 0) {
            printf("%ld -- %d (%d%%):\t %ld\n",
                   time(0) - prev_time,
                   i,
                   (i * 100) / 1000000,
                   compute_ratio(old_line, size));


            // opens it in append mode
            output = fopen("output.txt", "a");

            fprintf(output, "%ld -- %d (%d%%):\t %ld\n",
                    time(0) - prev_time,
                   i,
                   (i * 100) / 1000000,
                   compute_ratio(old_line, size));

            fclose(output);

            prev_time = time(0);
        }


        get_next_line (old_line, new_line, size, t_args, chunk_size, n_chunks);
        birthday(old_line, new_line, size);
    }
    free(t_args);
    free(old_line);
    free(new_line);
}
