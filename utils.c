#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lab5.h"

extern Node *g_root;

/* TODO 29: Implement check_integrity
 * Use BFS to verify tree structure:
 * - Question nodes must have both yes and no children (not NULL)
 * - Leaf nodes (isQuestion == 0) must have NULL children
 * 
 * Return 1 if valid, 0 if invalid
 * 
 * Steps:
 * 1. Return 1 if g_root is NULL (empty tree is valid)
 * 2. Initialize queue and enqueue root with id=0
 * 3. Set valid = 1
 * 4. While queue not empty:
 *    - Dequeue node
 *    - If node->isQuestion:
 *      - Check if yes == NULL or no == NULL
 *      - If so, set valid = 0 and break
 *      - Otherwise, enqueue both children
 *    - Else (leaf node):
 *      - Check if yes != NULL or no != NULL
 *      - If so, set valid = 0 and break
 * 5. Free queue and return valid
 */
int check_integrity() {
    // Empty tree is considered valid
    if (g_root == NULL) {
        return 1;
    }

    // Allocate and initialize a Queue for BFS
    Queue *q = malloc(sizeof(Queue));
    q_init(q);

    int id = 0; // id values enqueued are unused by integrity logic but required by Queue API
    q_enqueue(q, g_root, id); // start BFS from root

    int valid = 1; // assume valid until a rule violation is found
    Node *dequeueNode;
    int dequeueId;

    // BFS loop: check each node's invariants
    while (!q_empty(q)) {
        q_dequeue(q, &dequeueNode, &dequeueId);

        if (dequeueNode->isQuestion) {
            // Question nodes must have both children
            if (dequeueNode->yes == NULL || dequeueNode->no == NULL) {
                valid = 0; // found invalid question node
                break;
            } else {
                // Enqueue both children for further checking
                q_enqueue(q, dequeueNode->yes, ++id);
                q_enqueue(q, dequeueNode->no, ++id);
            }
        } else {
            // Leaf nodes must not have children
            if (dequeueNode->yes != NULL || dequeueNode->no != NULL) {
                valid = 0; // found invalid leaf node
                break;
            }
        }
    }

    // Clean up queue resources
    q_free(q);
    free(q);
    q = NULL;

    return valid;
}

typedef struct PathNode {
    Node *treeNode;
    struct PathNode *parent;
    int viaYes;  /* 1 if reached via yes, 0 if via no, -1 for root */
} PathNode;

/* TODO 30: Implement find_shortest_path (OPTIONAL CHALLENGE)
 * Find the shortest distinguishing path between two animals
 * 
 * This is an optional challenge problem. If you want to attempt it:
 * 
 * Steps:
 * 1. Use BFS to find paths to both animals
 * 2. Build PathNode structures that track parent and direction
 * 3. Build arrays of PathNodes from each animal to root
 * 4. Find the Lowest Common Ancestor (LCA)
 * 5. Print the questions that distinguish them
 * 
 * Algorithm:
 * - Use BFS with a PathNode array to track the path taken
 * - For each node visited, record its parent and which branch (yes/no)
 * - Once both animals are found, trace back to root for each
 * - Find where the paths diverge (LCA)
 * - Print the distinguishing questions from LCA to each animal
 */
void find_shortest_path(const char *animal1, const char *animal2) {
    if (g_root == NULL) return;
    // This optional function is not implemented in the starter code.
    // The goal is to perform two BFS searches that track parents and the
    // edge direction (yes/no) used to reach each node, then find the
    // lowest common ancestor (LCA) of the two animal leaf nodes. Once the
    // LCA is identified you can print the questions along the branches
    // from the LCA to each animal to show the distinguishing path.

    // Implementation notes / high-level plan (not executed here):
    // 1. BFS the tree storing for each visited Node a PathNode containing
    //    pointer to parent PathNode and viaYes flag.
    // 2. When animals are found, follow parent pointers to build their
    //    full paths to root.
    // 3. Compare the two paths to find the first common node (LCA).
    // 4. Print the sequence of questions from LCA to each animal which
    //    distinguishes them.

    printf("find_shortest_path not yet implemented\n");
}
