#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lab5.h"

extern Node *g_root;

#define MAGIC 0x41544C35  /* "ATL5" */
#define VERSION 1
#define MAX_TEXT_LEN 10000

typedef struct {
    Node *node;
    int id;
} NodeMapping;

/* TODO 27: Implement save_tree
 * Save the tree to a binary file using BFS traversal
 * 
 * Binary format:
 * - Header: magic (4 bytes), version (4 bytes), nodeCount (4 bytes)
 * - For each node in BFS order:
 *   - isQuestion (1 byte)
 *   - textLen (4 bytes)
 *   - text (textLen bytes, no null terminator)
 *   - yesId (4 bytes, -1 if NULL)
 *   - noId (4 bytes, -1 if NULL)
 * 
 * Steps:
 * 1. Return 0 if g_root is NULL
 * 2. Open file for writing binary ("wb")
 * 3. Initialize queue and NodeMapping array
 * 4. Use BFS to assign IDs to all nodes:
 *    - Enqueue root with id=0
 *    - Store mapping[0] = {g_root, 0}
 *    - While queue not empty:
 *      - Dequeue node and id
 *      - If node has yes child: add to mappings, enqueue with new id
 *      - If node has no child: add to mappings, enqueue with new id
 * 5. Write header (magic, version, nodeCount)
 * 6. For each node in mapping order:
 *    - Write isQuestion, textLen, text bytes
 *    - Find yes child's id in mappings (or -1)
 *    - Find no child's id in mappings (or -1)
 *    - Write yesId, noId
 * 7. Clean up and return 1 on success
 */
int find_id_for_node(NodeMapping *mapping, int count, Node *node) { 
    // If caller asks for ID of a NULL child, represent as -1 in file format
    if (node == NULL) {
        return -1;
    }

    // Linear search through mapping array to find the node's assigned id
    for (int i = 0; i < count; i++) {
        if (mapping[i].node == node) {
            return mapping[i].id;
        }
    }

    // Not found: return -1 (should not occur if mapping was built correctly)
    return -1;
}

int save_tree(const char *filename) {
    if (g_root == NULL) {
        return 0;
    }

    FILE* fileptr = NULL;
    NodeMapping* mapping = NULL;
    Queue* q = NULL;
    int success = 0;

    // Open file for binary writing. Using "wb" truncates/creates the file.
    fileptr = fopen(filename, "wb");
    if (fileptr == NULL) {
        perror("[save_tree] Failed to open file");
        goto save_error;
    }

    /* --- First BFS pass: count nodes --- */
    // First pass: BFS traversal to count nodes so we can allocate mapping array
    q = malloc(sizeof(Queue));
    if (q == NULL) {
        perror("[save_tree] Failed to allocate queue");
        goto save_error;
    }
    q_init(q);

    Node *currNode = NULL;
    int tmpId = 0;   // id returned by queue (not used in counting pass)
    int counted = 0; // total nodes seen

    // Start BFS from root
    q_enqueue(q, g_root, 0);
    while (!q_empty(q)) {
        q_dequeue(q, &currNode, &tmpId);
        counted++; // increment node count for each dequeued node

        // Enqueue children if present (id placeholder not used in counting)
        if (currNode->yes != NULL) q_enqueue(q, currNode->yes, 0);
        if (currNode->no  != NULL) q_enqueue(q, currNode->no,  0);
    }


    /* allocate mapping */
    // Allocate mapping array sized to the node count discovered above
    mapping = malloc(counted * sizeof(NodeMapping));
    if (mapping == NULL) {
        perror("[save_tree] Failed to allocate mapping");
        goto save_error;
    }

    /* Rebuild BFS order */
    // Second pass: rebuild BFS ordering and record node -> id mapping
    q_free(q);
    free(q);
    q = malloc(sizeof(Queue));
    if (q == NULL) {
        perror("[save_tree] Failed to allocate queue (2nd pass)");
        goto save_error;
    }
    q_init(q);

    int nodeCount = counted; // total nodes available
    int mIdx = 0;            // mapping index currently assigned

    // Seed mapping[0] with root
    q_enqueue(q, g_root, 0);
    mapping[0].node = g_root;
    mapping[0].id = 0;

    // Perform BFS and assign sequential ids to each newly discovered child
    while (!q_empty(q)) {
        int dequeueId = 0;
        q_dequeue(q, &currNode, &dequeueId);

        // If yes-child exists, assign next id and enqueue it
        if (currNode->yes != NULL) {
            mIdx++;
            mapping[mIdx].node = currNode->yes;
            mapping[mIdx].id = mIdx;
            q_enqueue(q, currNode->yes, mIdx);
        }

        // If no-child exists, assign next id and enqueue it
        if (currNode->no != NULL) {
            mIdx++;
            mapping[mIdx].node = currNode->no;
            mapping[mIdx].id = mIdx;
            q_enqueue(q, currNode->no, mIdx);
        }
    }

    // Sanity check: ensure we assigned exactly nodeCount ids
    if (mIdx + 1 != nodeCount) {
        goto save_error;
    }

    /* --- Write header --- */
    uint32_t magic_val = MAGIC;
    uint32_t version_val = VERSION;
    uint32_t count_val = (uint32_t)nodeCount;

    // Write the header: magic, version, and number of nodes
    if (fwrite(&magic_val, sizeof(uint32_t), 1, fileptr) != 1) { goto save_error; }
    if (fwrite(&version_val, sizeof(uint32_t), 1, fileptr) != 1) { goto save_error; }
    if (fwrite(&count_val, sizeof(uint32_t), 1, fileptr) != 1) { goto save_error; }

    /* --- Write nodes --- */
    for (int i = 0; i < nodeCount; i++) {
        Node* node = mapping[i].node;

        // For each mapped node, write its record in BFS order
        uint8_t is_q = (uint8_t)node->isQuestion;            // 1 byte: question flag
        uint32_t textLen = (uint32_t)strlen(node->text);    // 4 bytes: text length

        // Map child pointers to their assigned IDs (or -1 if NULL)
        int32_t yesId = find_id_for_node(mapping, nodeCount, node->yes);
        int32_t noId  = find_id_for_node(mapping, nodeCount, node->no);

        // Write flag and text length
        if (fwrite(&is_q, sizeof(uint8_t), 1, fileptr) != 1) { goto save_error; }
        if (fwrite(&textLen, sizeof(uint32_t), 1, fileptr) != 1) { goto save_error; }

        // Write text bytes (no null terminator stored in file)
        if (textLen > 0) {
            if (fwrite(node->text, sizeof(char), textLen, fileptr) != textLen) { goto save_error; }
        }

        // Write yesId and noId as 32-bit signed integers
        if (fwrite(&yesId, sizeof(int32_t), 1, fileptr) != 1) { goto save_error; }
        if (fwrite(&noId,  sizeof(int32_t), 1, fileptr) != 1) { goto save_error; }
    }

    success = 1;

save_error:
    if (fileptr != NULL) fclose(fileptr);
    if (q != NULL) { q_free(q); free(q); }
    if (mapping != NULL) free(mapping);
    return success;
}

/* TODO 28: Implement load_tree
 * Load a tree from a binary file and reconstruct the structure
 * 
 * Steps:
 * 1. Open file for reading binary ("rb")
 * 2. Read and validate header (magic, version, count)
 * 3. Allocate arrays for nodes and child IDs:
 *    - Node **nodes = calloc(count, sizeof(Node*))
 *    - int32_t *yesIds = calloc(count, sizeof(int32_t))
 *    - int32_t *noIds = calloc(count, sizeof(int32_t))
 * 4. Read each node:
 *    - Read isQuestion, textLen
 *    - Validate textLen (e.g., < 10000)
 *    - Allocate and read text string (add null terminator!)
 *    - Read yesId, noId
 *    - Validate IDs are in range [-1, count)
 *    - Create Node and store in nodes[i]
 * 5. Link nodes using stored IDs:
 *    - For each node i:
 *      - If yesIds[i] >= 0: nodes[i]->yes = nodes[yesIds[i]]
 *      - If noIds[i] >= 0: nodes[i]->no = nodes[noIds[i]]
 * 6. Free old g_root if not NULL
 * 7. Set g_root = nodes[0]
 * 8. Clean up temporary arrays
 * 9. Return 1 on success
 * 
 * Error handling:
 * - If any read fails or validation fails, goto load_error
 * - In load_error: free all allocated memory and return 0
 */
int load_tree(const char *filename) {
    // Pointers to be allocated and used during loading
    FILE *fileptr = NULL;
    Node **nodes = NULL;           // array to store newly created Node pointers
    int32_t *yesIds = NULL;        // array to store yes child IDs (to link later)
    int32_t *noIds = NULL;         // array to store no child IDs (to link later)
    char *text_buffer = NULL;      // temporary buffer for reading node text
    uint32_t count = 0;            // number of nodes in the file
    int success = 0;               // success flag: 0 = fail, 1 = success

    // Open the file for binary reading
    fileptr = fopen(filename, "rb");
    if (fileptr == NULL) {
        perror("[load_tree] Could not open file");
        goto cleanup;
    }

    // Read and validate the header (magic, version, count)
    uint32_t magic, version;
    if (fread(&magic, sizeof(uint32_t), 1, fileptr) != 1) goto cleanup;
    if (fread(&version, sizeof(uint32_t), 1, fileptr) != 1) goto cleanup;
    if (fread(&count, sizeof(uint32_t), 1, fileptr) != 1) goto cleanup;

    // Verify magic and version match what we saved
    if (magic != MAGIC || version != VERSION) {
        goto cleanup;
    }

    // Special case: if the file contains no nodes (empty tree)
    if (count == 0) {
        if (g_root != NULL) free_tree(g_root); // free old tree if present
        g_root = NULL;                         // set global to empty
        success = 1;
        goto cleanup;
    }

    // Allocate arrays for the nodes and their child IDs
    // calloc zero-initializes, which is safe for pointers and IDs
    nodes = calloc(count, sizeof(Node*));
    yesIds = calloc(count, sizeof(int32_t));
    noIds = calloc(count, sizeof(int32_t));
    if (!nodes || !yesIds || !noIds) goto cleanup;

    // Read each node record from the file
    for (uint32_t i = 0; i < count; i++) {
        uint8_t is_q;              // question flag (1 = question, 0 = leaf)
        uint32_t textLen;          // length of the text string
        int32_t yesId, noId;       // IDs of child nodes in the loaded array

        // Read the question flag and text length
        if (fread(&is_q, sizeof(uint8_t), 1, fileptr) != 1) goto cleanup;
        if (fread(&textLen, sizeof(uint32_t), 1, fileptr) != 1) goto cleanup;

        // Validate text length is reasonable
        if (textLen > MAX_TEXT_LEN) goto cleanup;

        // Allocate buffer for the text (add 1 for null terminator we'll add)
        text_buffer = malloc(textLen + 1);
        if (text_buffer == NULL) goto cleanup;

        // Read textLen bytes of the text string from file
        if (fread(text_buffer, sizeof(char), textLen, fileptr) != textLen) goto cleanup;

        // Null-terminate the string (file doesn't store the null terminator)
        text_buffer[textLen] = '\0';

        // Read the child node IDs (or -1 for NULL pointers)
        if (fread(&yesId, sizeof(int32_t), 1, fileptr) != 1) goto cleanup;
        if (fread(&noId, sizeof(int32_t), 1, fileptr) != 1) goto cleanup;

        // Validate IDs are in the valid range: -1 (no child) or 0 to count-1
        if (yesId < -1 || yesId >= (int32_t)count) goto cleanup;
        if (noId < -1 || noId >= (int32_t)count) goto cleanup;

        // Create the Node: if is_q==1 it's a question, else it's an animal (leaf)
        nodes[i] = is_q ? create_question_node(text_buffer)
                        : create_animal_node(text_buffer);
        if (nodes[i] == NULL) goto cleanup;

        // Clean up temp buffer for next iteration
        free(text_buffer);
        text_buffer = NULL;

        // Store the child IDs for the linking phase (next loop)
        yesIds[i] = yesId;
        noIds[i] = noId;
    }

    // Second phase: reconnect child pointers using stored IDs
    // We do this separately so all nodes exist before linking
    for (uint32_t i = 0; i < count; i++) {
        // If yesId is not -1, the yes child is nodes[yesIds[i]]
        if (yesIds[i] != -1) nodes[i]->yes = nodes[yesIds[i]];

        // If noId is not -1, the no child is nodes[noIds[i]]
        if (noIds[i] != -1) nodes[i]->no  = nodes[noIds[i]];
    }

    // Replace the old global tree root with the newly loaded one
    if (g_root != NULL) free_tree(g_root);
    g_root = nodes[0]; // node[0] is the root by BFS ordering

    // Mark success so cleanup code doesn't free the newly created nodes
    success = 1;

cleanup:
    // Close file if open
    if (fileptr) fclose(fileptr);

    // Free the temporary text buffer if allocated
    if (text_buffer) free(text_buffer);

    // Free the ID arrays (always safe to free)
    if (yesIds) free(yesIds);
    if (noIds) free(noIds);

    // If we failed, free all nodes that were created (success == 0)
    // This prevents a memory leak on error; on success we keep the nodes
    if (nodes && !success) {
        for (uint32_t i = 0; i < count; i++) {
            if (nodes[i]) free_tree(nodes[i]);
        }
    }

    // Free the node array itself
    if (nodes) free(nodes);

    return success;
}
