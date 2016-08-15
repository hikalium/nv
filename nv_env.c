
#include "nv.h"

struct NV_ENV {
	NV_LangDef *langDef;
	NV_VariableSet *varSet;
	NV_Pointer termRoot;
	int autoPrintValue;
	int endFlag;
};

NV_Env *NV_allocEnv()
{
	NV_Env *t;

	t = NV_malloc(sizeof(NV_Env));
	//
	t->langDef = NULL;
	t->termRoot = NV_NullPointer;

	return t;
}

int NV_Env_setVarSet(NV_Pointer env, NV_VariableSet *vs)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return 1;
	envp->varSet = vs;
	return 0;
}

NV_VariableSet *NV_Env_getVarSet(NV_Pointer env)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return NULL;
	return envp->varSet;
}

int NV_Env_setLangDef(NV_Pointer env, NV_LangDef *ld)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return 1;
	envp->langDef = ld;
	return 0;
}

NV_LangDef *NV_Env_getLangDef(NV_Pointer env)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return NULL;
	return envp->langDef;
}

int NV_Env_setAutoPrintValueEnabled(NV_Pointer env, int b)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return 1;
	envp->autoPrintValue = b;
	return 0;
}

int NV_Env_getAutoPrintValueEnabled(NV_Pointer env)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return 1;
	return envp->autoPrintValue;
}

int NV_Env_setEndFlag(NV_Pointer env, int b)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return 1;
	envp->endFlag = b;
	return 0;
}

int NV_Env_getEndFlag(NV_Pointer env)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return 1;
	return envp->endFlag;
}

NV_Pointer NV_Env_getTermRoot(NV_Pointer env)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return NV_NullPointer;
	return envp->termRoot;
}
