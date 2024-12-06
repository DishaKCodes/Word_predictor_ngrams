#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdbool.h>

#define MAX_WORDS 3
#define MAX_EDIT_DISTANCE 0.3
#define MAX_TOKEN_LEN 100
#define DELIMS " .,/?;:{}[]~`!|$%&*()_-+=^\'\"\t\n"

//structure for trie node
typedef struct trie_node {
    struct trie_node *children[26]; 
    int count;                     
    bool isEndOfWord;              
} trie_node;

//structure for the trie data structure
typedef struct trie{
    trie_node * root;
    long long int total_unigram_count;
}trie;


typedef struct {
    char word[100];
    double prob;
    float distance;
} word_element;

//structure for the priority queue
typedef struct {
    word_element words_collection[MAX_WORDS];
    int size;
} priority_Q;

//node structure for stack implementation using a linked list
typedef struct node{
    char token[MAX_TOKEN_LEN]; 
    struct node * next;
}node;

typedef struct{
    node * top;
    int size;
}stack;


//function to initialise the priority queue
void init_priority_Q(priority_Q * pq);

//function to initialsie the trie data structure
void init_trie(trie * T);

//helper function for get a trie node
trie_node * get_node();

//function to insert a unigram into trie
void insert_word(trie * T, const char * word, int count);

//file handling function : extracts the word and
//their counts from the file and inserts in the trie
void process_csv_file(const char *filename, trie *T);

//helper function : to calculate the Markov probability
double unigram_prob(int count, int total);

//function to check if a given token exits in the trie
double is_unigram(trie T, const char *token, priority_Q *pq);

//function to check if a prefix is contained by any word in the trie 
int is_prefix(trie T, const char * token, priority_Q * result);

//trie function : a recursive function to add a word to the priority queue
//if it encounters a IsEndOfWord = true.
void collect_words(trie_node * curr, priority_Q * result, char * word, int level, long long int total_unigram_count);

//priority queue function : inserts in the priority queue in a sorted manner based on the probability
void insert_pq(priority_Q *result, const char *word, double prob, float dist);

//helper function : find the minimum out of three numbers
int min3(int a, int b, int c);

//function to calculate the edit distance for the Levenshtein algorithm 
float edit_distance(const char* word1, const char* word2);

//IS THIS FUNCTION EVEN USED ANYWHERE
void sort_by_edits(priority_Q *pq);

//this function performs fuzzy matching to find the potential spell corrections.
void collect_fuzzy(trie_node * p, priority_Q *pq, const char * token, char * curr_word, int level, long long int total_unigram_count);

//helper function to check if a potential spellling corrected words (fuzzy matches) were found or not
int is_fuzzymatch(trie T, const char * token, priority_Q * result);

//prepares the search set (used for word prediction)...by checking if the last two tokens can be validated using the trie
void backoff(trie T, const char *token1, const char *token2, word_element search_set[], priority_Q *result1, priority_Q *result2);

//performs search, prefix matching and then fuzzy macthing to check if token is valid for word prediction
word_element validate(trie T, const char * token, priority_Q * result);

//debugging fucntion : to display all the words present in the trie
void display_trie_helper(trie_node *node, char *prefix, int level);
void display_trie(trie T);

//helper function : init stcak to store the tokens in the user string
void init_stack(stack *s);

//helper function : to push tokens extracted from the user string to the stack
void push(stack * s, char * token);

//helper function : to check if a token is a valid word
int is_word(const char *token);

//helper function to check if stack is empty
int is_empty(stack s);

//helper function to pop a token from stack
char * pop(stack * s);

//helper function : to make a string smallcase
void to_lower(char * input_string);

//helper function : to check if an encountered character is an delimiter
int is_delim(char ch);

//tokenises the user entered string baed on the defined set of delimiters
void tokenize_user_string(char *user_string, stack *s);

//this is the function for processing the words NOT used for word prediction.
//basically spell checks and processes the entire string
//the predicted phrase would be concatenated to the string returned by this function 
char* word_processor(stack *s1, stack *s2, trie *T);

//


#endif

