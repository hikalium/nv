//
// Node Internal
//

struct NV_NODE {
	NV_ID id;
	const void *data;
	NV_Node *prev;
	NV_Node *next;
	NV_NodeType type;
	int size;	// size of data, bytes.
	//
	const NV_Node *relCache; // link from this node. recently referenced.
};

