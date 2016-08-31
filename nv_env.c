
#include "nv.h"

struct NV_ENV {
	NV_Pointer lang;
	int autoPrintValue;
	int endFlag;
};

//
// NV_Element
//

NV_Env *NV_E_allocEnv()
{
	NV_Env *t;

	t = NV_malloc(sizeof(NV_Env));
	//
	t->lang = NV_NullPointer;

	return t;
}

void NV_E_free_internal_Env(NV_Pointer item)
{
	NV_Env *envp = NV_E_getRawPointer(item, EEnv);
	NV_E_free(&envp->lang);	
}

//
// NV_Env
//

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

