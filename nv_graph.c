#include "nv.h"
//
// Graph
//
NV_Node nodeRoot;

void NV_Graph_addStaticNode(const NV_ID *id, const char *s){
	NV_Node_createWithID(id);
	NV_Node_setStrToID(id, s);
	NV_Node_createRelation(&NODEID_NV_STATIC_ROOT, &NODEID_NULL, id);
}

void NV_Graph_init()
{
	srand(time(NULL));
	//
	nodeRoot.prev = NULL;
	nodeRoot.next = NULL;
	nodeRoot.data = NULL;
	nodeRoot.type = kNone;
	//
	NV_Node_createWithID(&NODEID_NV_STATIC_ROOT);
	NV_Node_setStrToID(&NODEID_NV_STATIC_ROOT, "NV_StaticRoot");
	NV_Node_retain(&NODEID_NV_STATIC_ROOT);
	//
	NV_Graph_addStaticNode(&NODEID_NULL, "NullElement");
	NV_Graph_addStaticNode(&NODEID_TREE_TYPE_ARRAY, "TreeType(Array)");
	NV_Graph_addStaticNode(&NODEID_TREE_TYPE_VARIABLE, "TreeType(Variable)");
	NV_Graph_addStaticNode(&NODEID_TREE_TYPE_OP, "TreeType(Op)");
	//
	NV_Graph_addStaticNode(&RELID_TREE_TYPE, "relTreeType");
	NV_Graph_addStaticNode(&RELID_ARRAY_NEXT, "relArrayNext");
	NV_Graph_addStaticNode(&RELID_VARIABLE_DATA, "relVariableData");
	NV_Graph_addStaticNode(&RELID_POINTER_TARGET, "relPointerTarget");
	NV_Graph_addStaticNode(&RELID_OP_PRECEDENCE, "relPointerTarget");
	NV_Graph_addStaticNode(&RELID_OP_FUNC, "relPointerTarget");
}


void NV_Graph_dump()
{
	NV_Node *n;
	//
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_dump(n); putchar('\n');
	}
}

/*
void NV_Graph_dumpToFile(const char *fname)
{
	FILE *fp = fopen(fname, "wb");
	NV_Node *n;
	if(!fp) return;
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_dump(n); putchar('\n');
	}
	fclose(fp);
}
*/
void NV_Graph_dumpToFile(FILE *fp)
{
	NV_Node *n;
	if(!fp) return;
	for(n = nodeRoot.next; n; n = n->next){
		NV_Node_fdump(fp, n); fputc('\n', fp);
	}
}

void NV_Graph_restoreFromFile(FILE *fp)
{
	static char sTable[0x100];
	char s[4096], *p;
	int n, i;
	NV_ID id;
	NV_ID from, rel, to;
	for(i = 0; i < 10; i++){
		sTable[i + '0'] = i;
	}
	for(i = 0; i < 6; i++){
		sTable[i + 'a'] = i + 0x0a;
		sTable[i + 'A'] = i + 0x0a;
	}

	while(fgets(s, sizeof(s), fp)){
		if(NV_ID_setFromString(&id, s)){
			printf("Invalid id format.\n");
			return;
		}
		NV_ID_dumpIDToFile(&id, stdout);
		putchar(' ');
		NV_Node_createWithID(&id);
		//
		p = &s[32];
		n = strtol(p, &p, 10);
		switch(n){
			case kString:
				n = strtol(p, &p, 10);
				p++;
				for(i = 0; i < n - 1; i++){
					s[i] = sTable[(int)*(p++)];
					s[i] <<= 4;
					s[i] |= sTable[(int)*(p++)];
				}
				s[i] = 0;
				printf("str(%d) = %s\n", n, s);
				NV_Node_setStrToID(&id, s);
				break;
			case kInteger:
				n = strtol(p, &p, 10);
				printf("int(%d) = ", n);
				//
				n = strtol(p, &p, 16);
				printf("%d\n", n);
				NV_Node_setInt32ToID(&id, n);
				break;
			case kRelation:
				NV_ID_setFromString(&from, &p[1]);
				NV_ID_setFromString(&rel, &p[ 1 + 32 + 1]);
				NV_ID_setFromString(&to, &p[1 + 32 + 1 + 32 + 1]);
				printf("rel ");
				printf(" ");
				NV_ID_dumpIDToFile(&from, stdout);
				printf(" ");
				NV_ID_dumpIDToFile(&rel, stdout);
				printf(" ");
				NV_ID_dumpIDToFile(&to, stdout);
				printf("\n");
				NV_Node_setRelation(&id, &from, &rel, &to);
				break;
		}
	}
}

