
#include "nv.h"

struct NV_ENV {
	NV_Pointer lang;
	NV_Pointer varRoot;
	NV_Pointer termRoot;
	int autoPrintValue;
	int endFlag;
};

NV_Env *NV_E_allocEnv()
{
	NV_Env *t;

	t = NV_malloc(sizeof(NV_Env));
	//
	t->lang = NV_NullPointer;
	t->varRoot = NV_Dict_allocRoot();
	t->termRoot = NV_List_allocRoot();

	return t;
}

NV_Pointer NV_Env_getVarRoot(NV_Pointer env)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return NV_NullPointer;
	return envp->varRoot;
}

int NV_Env_setLang(NV_Pointer env, NV_Pointer lang)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return 1;
	envp->lang = lang;
	return 0;
}

NV_Pointer NV_Env_getLang(NV_Pointer env)
{
	NV_Env *envp = NV_E_getRawPointer(env, EEnv);
	if(!envp) return NV_NullPointer;
	return envp->lang;
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

