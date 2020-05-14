#include <unordered_map>
#include "config_var.h"
#include "multimode.h"

std::unordered_set<MMConfigVarBase *> &MMConfigVarBase::GetModeVars(ModeID mode)
{
	static std::unordered_map<ModeID, std::unordered_set<MMConfigVarBase *>> map;
	return map[mode];
}
