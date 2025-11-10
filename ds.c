#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lab5.h"

/* ========== Node Functions ========== */

/* TODO 1: Implement create_question_node
 * - Allocate memory for a Node structure 
 * - Use strdup() to copy the question string (heap allocation)
 * - Set isQuestion to 1
 * - Initialize yes and no pointers to NULL
 * - Return the new node
 */
Node *create_question_node(const char *question) {
    // Validate input: if caller passed NULL, we can't create a node
    if (question == NULL) {
        return NULL;
    }
    // Allocate memory for the Node structure on the heap
    Node *initialNode = malloc(sizeof(Node));
    if (initialNode == NULL) { // allocation failed
        return NULL;
    }
    // Duplicate the question string so the node owns its own copy
    initialNode->text = strdup(question);
    if (initialNode->text == NULL) { // strdup failed
        free(initialNode);            // avoid memory leak
        return NULL;
    }
    // Mark this node as a question (non-leaf)
    initialNode->isQuestion = 1;
    // Initialize child pointers to NULL; children will be assigned later
    initialNode->yes = NULL;
    initialNode->no = NULL;
    // Return the newly-created question node
    return initialNode;
}

/* TODO 2: Implement create_animal_node
 * - Similar to create_question_node but set isQuestion to 0
 * - This represents a leaf node with an animal name
 */
Node *create_animal_node(const char *animal) {
    // If no animal string was provided, fail early
    if (animal == NULL) {
        return NULL;
    }
    // Allocate a Node to represent the leaf (animal)
    Node *initialNode = malloc(sizeof(Node));
    if (initialNode == NULL) { // allocation failure
        return NULL;
    }
    // Copy the animal name so the node owns the string
    initialNode->text = strdup(animal);
    if (initialNode->text == NULL) { // strdup failed
        free(initialNode);
        return NULL;
    }
    // Mark this node as a leaf (not a question)
    initialNode->isQuestion = 0;
    // Leaves have no children
    initialNode->yes = NULL;
    initialNode->no = NULL;
    return initialNode;
}

/* TODO 3: Implement free_tree (recursive)
 * - This is one of the few recursive functions allowed
 * - Base case: if node is NULL, return
 * - Recursively free left subtree (yes)
 * - Recursively free right subtree (no)
 * - Free the text string
 * - Free the node itself
 * IMPORTANT: Free children before freeing the parent!
 */
void free_tree(Node *node) {
    // If node is NULL there's nothing to free (base case)
    if (node == NULL) {
        return;
    }
    // Recursively free the 'yes' subtree first
    free_tree(node->yes); 
    // Then recursively free the 'no' subtree
    free_tree(node->no); 
    // Free the string owned by this node
    free(node->text);
    // Finally free the node itself
    free(node);
} 

/* TODO 4: Implement count_nodes (recursive)
 * - Base case: if root is NULL, return 0
 * - Return 1 + count of left subtree + count of right subtree
 */
int count_nodes(Node *root) {
    // Base case: empty subtree contributes 0 to the count
    if (root == NULL) {
        return 0;
    }
    // Count this node (1) plus nodes in both subtrees (recursive)
    return 1 + count_nodes(root->yes) + count_nodes(root->no);
}

/* ========== Frame Stack (for iterative tree traversal) ========== */

/* TODO 5: Implement fs_init
 * - Allocate initial array of frames (start with capacity 16)
 * - Set size to 0
 * - Set capacity to 16
 */
void fs_init(FrameStack *s) {
    // Ensure the stack pointer is valid
    if (s == NULL) {
        return;
    }
    // Allocate an initial array of 16 frames
    s->frames = malloc(16 * sizeof(Frame));
    s->capacity = 16; // total capacity
    s->size = 0;      // no frames yet
    // Initialize contents to safe defaults
    for (int i = 0; i < 16; i++) {
        s->frames[i].node = NULL;
        s->frames[i].answeredYes = -1;
    }
}

/* TODO 6: Implement fs_push
 * - Check if size >= capacity
 *   - If so, double the capacity and reallocate the array
 * - Store the node and answeredYes in frames[size]
 * - Increment size
 */
void fs_push(FrameStack *s, Node *node, int answeredYes) {
    // Defensive checks
    if (s == NULL || s->frames == NULL) {
        return;
    }
    // If capacity, grow the array by doubling
    if (s->size >= s->capacity) {
        s->capacity = 2 * s->capacity;
        s->frames = realloc(s->frames, s->capacity * sizeof(Frame));
    }
    // Store the frame at the current size index
    s->frames[s->size].node = node;
    s->frames[s->size].answeredYes = answeredYes;
    // Increment size bc/ pushed frame
    s->size = s->size + 1;
}

/* TODO 7: Implement fs_pop
 * - Decrement size
 * - Return the frame at frames[size]
 * Note: No need to check if empty - caller should use fs_empty() first
 */
Frame fs_pop(FrameStack *s) {
    // Provide a safe dummy for empty/invalid calls
    Frame dummy = {NULL, -1};
    // If stack is invalid or empty, return dummy
    if (s == NULL || s->size == 0) {
        return dummy;
    }
    // Decrement size first to index the top element
    s->size = s->size - 1;
    // Return the frame that was on top of the stack
    return s->frames[s->size];
}

/* TODO 8: Implement fs_empty
 * - Return 1 if size == 0, otherwise return 0
 */
int fs_empty(FrameStack *s) {
    // Empty if stack is NULL or size is zero
    if (s == NULL || s->size == 0) {
        return 1;
    } else {
        return 0;
    }
}

/* TODO 9: Implement fs_free
 * - Free the frames array
 * - Set frames pointer to NULL
 * - Reset size and capacity to 0
 */
void fs_free(FrameStack *s) {
    // Free internal frame array and reset fields
    if (s == NULL) {
        return;
    }

    free(s->frames);
    s->frames = NULL;
    s->size = 0;
    s->capacity = 0;
}

/* ========== Edit Stack (for undo/redo) ========== */

/* TODO 10: Implement es_init
 * Similar to fs_init but for Edit structs
 */
void es_init(EditStack *s) {
    // Initialize an EditStack similarly to FrameStack
    if (s == NULL) {
        return;
    }
    s->edits = malloc(16 * sizeof(Edit));
    s->capacity = 16;
    s->size = 0;
}

/* TODO 11: Implement es_push
 * Similar to fs_push but for Edit structs
 * - Check capacity and resize if needed
 * - Add edit to array and increment size
 */
void es_push(EditStack *s, Edit e) {
    // Defensive checks
    if (s == NULL || s->edits == NULL) {
        return;
    }
    // Grow array when needed
    if (s->size >= s->capacity) {
        s->capacity = 2 * s->capacity;
        s->edits = realloc(s->edits, s->capacity * sizeof(Edit));
    }
    // Append the edit and increment size
    s->edits[s->size] = e;
    s->size = s->size + 1;
}

/* TODO 12: Implement es_pop
 * Similar to fs_pop but for Edit structs
 */
Edit es_pop(EditStack *s) {
    // Return a dummy Edit for invalid/empty stacks
    Edit dummy = {0};
    if (s == NULL || s->size == 0) {
        return dummy;
    }
    // Decrement size and return the last pushed edit
    s->size = s->size - 1;
    return s->edits[s->size];
}

/* TODO 13: Implement es_empty
 * Return 1 if size == 0, otherwise 0
 */
int es_empty(EditStack *s) {
    if (s == NULL || s->size == 0) {
        return 1;
    } else {
        return 0;
    }
}

/* TODO 14: Implement es_clear
 * - Set size to 0 (don't free memory, just reset)
 * - This is used to clear the redo stack when a new edit is made
 */
void es_clear(EditStack *s) {
    // Reset the logical size to zero; keep allocated memory for reuse
    if (s == NULL) {
        return;
    }
    s->size = 0;
}
void es_free(EditStack *s) {
    if(s == NULL) {return;}
    free(s->edits);
    s->edits = NULL;
    s->size = 0;
    s->capacity = 0;
}
void free_edit_stack(EditStack *s) {
    es_free(s);
}

/* ========== Queue (for BFS traversal) ========== */

/* TODO 15: Implement q_init
 * - Set front and rear to NULL
 * - Set size to 0
 */
void q_init(Queue *q) {
    // Initialize queue pointers and size
    if (q == NULL) {
        return;
    }
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

/* TODO 16: Implement q_enqueue
 * - Allocate a new QueueNode
 * - Set its treeNode and id fields
 * - Set its next pointer to NULL
 * - If queue is empty (rear == NULL):
 *   - Set both front and rear to the new node
 * - Otherwise:
 *   - Link rear->next to the new node
 *   - Update rear to point to the new node
 * - Increment size
 */
void q_enqueue(Queue *q, Node *node, int id) {
    // Enqueue a new QueueNode at the tail of the list
    if (q == NULL) {
        return;
    }
    QueueNode *new = malloc(sizeof(QueueNode));
    new->treeNode = node; // store pointer to the tree node
    new->id = id;         // store the supplied id (used during mapping)
    new->next = NULL;     // new tail has no next
    // If queue empty, both front and rear point to new node
    if (q->rear == NULL) {
        q->front = new;
        q->rear = new;
    } else {
        // Otherwise append after rear and update rear
        q->rear->next = new;
        q->rear = new;
    }
    // Increment size counter
    q->size = q->size + 1;
}

/* TODO 17: Implement q_dequeue
 * - If queue is empty (front == NULL), return 0
 * - Save the front node's data to output parameters (*node, *id)
 * - Save front in a temp variable
 * - Move front to front->next
 * - If front is now NULL, set rear to NULL too
 * - Free the temp node
 * - Decrement size
 * - Return 1
 */
int q_dequeue(Queue *q, Node **node, int *id) {
    // Dequeue from the head of the queue and return its data
    if (q == NULL) {
        return 0; // invalid queue
    }
    // Empty queue -> nothing to dequeue
    if (q->front == NULL) {
        return 0;
    }
    // Return values to caller via out parameters
    *node = q->front->treeNode;
    *id = q->front->id;
    // Remove the front node from the list
    QueueNode *temp = q->front;
    q->front = q->front->next;
    // If queue is now empty, clear rear pointer
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);           // free removed QueueNode
    q->size = q->size - 1; // decrement size
    return 1;             // success
}

/* TODO 18: Implement q_empty
 * Return 1 if size == 0, otherwise 0
 */
int q_empty(Queue *q) {
    if (q == NULL || q->size == 0) {
        return 1;
    }
    return 0;
}

/* TODO 19: Implement q_free
 * - Dequeue all remaining nodes
 * - Use a loop with q_dequeue until queue is empty
 */
void q_free(Queue *q) {
    // Remove and free all remaining QueueNodes
    if (q == NULL) {
        return;
    }
    Node *dummyNode;
    int dummyId;
    while (!q_empty(q)) {
        q_dequeue(q, &dummyNode, &dummyId); // q_dequeue frees nodes internally
    }
}

/* ========== Hash Table ========== */

/* TODO 20: Implement canonicalize
 * Convert a string to canonical form for hashing:
 * - Convert to lowercase
 * - Keep only alphanumeric characters
 * - Replace spaces with underscores
 * - Remove punctuation
 * Example: "Does it meow?" -> "does_it_meow"
 * 
 * Steps:
 * - Allocate result buffer (strlen(s) + 1)
 * - Iterate through input string
 * - For each character:
 *   - If alphanumeric: add lowercase version to result
 *   - If whitespace: add underscore
 *   - Otherwise: skip it
 * - Null-terminate result
 * - Return the new string
 */
char *canonicalize(const char *s) {
    // Allocate a result buffer at most as long as the input (plus terminator)
    size_t len = strlen(s);
    char *result = malloc(len + 1);
    if (result == NULL) {
        return NULL; // allocation failed
    }
    int j = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        // If uppercase letter, convert to lowercase
        if (c >= 'A' && c <= 'Z') {
            result[j++] = (char)(c + ('a' - 'A'));
        }
        // Accept lowercase letters and digits unchanged
        else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
            result[j++] = (char)c;
        }
        // Replace whitespace with underscore to keep tokens readable
        else if (c == ' ') {
            result[j++] = '_';
        }
        // Silently skip all other punctuation characters
    }
    // Null-terminate the canonical string
    result[j] = '\0';
    return result;
}

/* TODO 21: Implement h_hash (djb2 algorithm)
 * unsigned hash = 5381;
 * For each character c in the string:
 *   hash = ((hash << 5) + hash) + c;  // hash * 33 + c
 * Return hash
 */
unsigned h_hash(const char *s) {
    //start with 5381
    unsigned int hash = 5381;
    // Iterate over each byte of the string and update hash = hash * 33 + c
    for (size_t i = 0; i < strlen(s); i++) {
        hash = ((hash << 5) + hash) + (unsigned char)s[i];
    }
    return hash;
}

/* TODO 22: Implement h_init
 * - Allocate buckets array using calloc (initializes to NULL)
 * - Set nbuckets field
 * - Set size to 0
 */
void h_init(Hash *h, int nbuckets) {
    if (h == NULL) {
        return;
    }
    // Allocate array of bucket pointers and zero-initialize them
    h->buckets = calloc(nbuckets, sizeof(Entry *));
    h->nbuckets = nbuckets;
    h->size = 0; // no entries yet
}

/* TODO 23: Implement h_put
 * Add animalId to the list for the given key
 * 
 * Steps:
 * 1. Compute bucket index: idx = h_hash(key) % nbuckets
 * 2. Search the chain at buckets[idx] for an entry with matching key
 * 3. If found:
 *    - Check if animalId already exists in the vals list
 *    - If yes, return 0 (no change) 
 *    - If no, add animalId to vals.ids array (resize if needed), return 1
 * 4. If not found:
 *    - Create new Entry with strdup(key)
 *    - Initialize vals with initial capacity (e.g., 4)
 *    - Add animalId as first element
 *    - Insert at head of chain (buckets[idx])
 *    - Increment h->size
 *    - Return 1
 */
int h_put(Hash *h, const char *key, int animalId) {
    // Compute the bucket index using the hash of the key
    int idx = h_hash(key) % h->nbuckets;

    // Walk the chain to see if the key already exists
    Entry *current = h->buckets[idx];
    while (current != NULL) {
        if (!strcmp(current->key, key)) {
            // Key found; check if animalId already present
            for (int i = 0; i < current->vals.count; i++) {
                if (current->vals.ids[i] == animalId) {
                    return 0; // no change needed
                }
            }
            // Need to append animalId to existing vals array; grow if full
            if (current->vals.capacity == current->vals.count) {
                int newCapacity;
                if (current->vals.capacity == 0) {
                    newCapacity = 4;
                    current->vals.capacity = 4;
                } else {
                    newCapacity = 2 * current->vals.capacity;
                }
                current->vals.ids = realloc(current->vals.ids, newCapacity * sizeof(int));
                if (current->vals.ids == NULL) {
                    return 0; // allocation failure
                }
                current->vals.capacity = newCapacity;
            }

            // Append id and update count
            current->vals.ids[current->vals.count] = animalId;
            current->vals.count++;
            return 1; // inserted into existing entry
        }
        current = current->next;
    }

    // Key not found: create a new Entry and insert at head of bucket chain
    Entry *newE = malloc(sizeof(Entry));
    if (newE == NULL) {
        return 0; // allocation failure
    }
    h->size++; // one more distinct key in the table
    newE->key = strdup(key); // copy the key string
    if (newE->key == NULL) {
        free(newE);
        h->size--;
        return 0;
    }
    // Initialize the value list with a small capacity
    newE->vals.capacity = 4;
    newE->vals.ids = malloc(newE->vals.capacity * sizeof(int));
    if (newE->vals.ids == NULL) {
        free(newE->key);
        free(newE);
        h->size--;
        return 0;
    }
    // Store the single id and set count
    newE->vals.ids[0] = animalId;
    newE->vals.count = 1;
    // Insert new entry at head of the chain
    Entry *oldHead = h->buckets[idx];
    h->buckets[idx] = newE;
    newE->next = oldHead;
    return 1; // success
}

/* TODO 24: Implement h_contains
 * Check if the hash table contains the given key-animalId pair
 * 
 * Steps:
 * 1. Compute bucket index
 * 2. Search the chain for matching key
 * 3. If found, search vals.ids array for animalId
 * 4. Return 1 if found, 0 otherwise
 */
int h_contains(const Hash *h, const char *key, int animalId) {
    // Locate bucket by hashing the key
    int idx = h_hash(key) % h->nbuckets;
    Entry *current = h->buckets[idx];
    while (current != NULL) {
        if (!strcmp(current->key, key)) {
            // Found the key; search its id list
            for (int i = 0; i < current->vals.count; i++) {
                if (current->vals.ids[i] == animalId) {
                    return 1; // pair exists
                }
            }
        }
        current = current->next;
    }
    return 0; // not found
}

/* TODO 25: Implement h_get_ids
 * Return pointer to the ids array for the given key
 * Set *outCount to the number of ids
 * Return NULL if key not found
 * 
 * Steps:
 * 1. Compute bucket index
 * 2. Search chain for matching key
 * 3. If found:
 *    - Set *outCount = vals.count
 *    - Return vals.ids
 * 4. If not found:
 *    - Set *outCount = 0
 *    - Return NULL
 */
int *h_get_ids(const Hash *h, const char *key, int *outCount) {
    // Return the ids array and its count for the given key
    int idx = h_hash(key) % h->nbuckets;
    Entry *current = h->buckets[idx];
    while (current != NULL) {
        if (!strcmp(current->key, key)) {
            *outCount = current->vals.count;
            return current->vals.ids; // caller must not free this
        }
        current = current->next;
    }
    // Key not present
    *outCount = 0;
    return NULL;
}

/* TODO 26: Implement h_free
 * Free all memory associated with the hash table
 * 
 * Steps:
 * - For each bucket:
 *   - Traverse the chain
 *   - For each entry:
 *     - Free the key string
 *     - Free the vals.ids array
 *     - Free the entry itself
 * - Free the buckets array
 * - Set buckets to NULL, size to 0
 */
void h_free(Hash *h) {
    // Free all entries in all buckets
    for (int i = 0; i < h->nbuckets; i++) {
        Entry *current = h->buckets[i];
        while (current != NULL) {
            Entry *next = current->next;
            free(current->key);       // free key string
            free(current->vals.ids);  // free ids array
            free(current);            // free Entry struct
            current = next;
        }
    }

    // Free the bucket array and reset state
    free(h->buckets);
    h->buckets = NULL;
    h->size = 0;
}
