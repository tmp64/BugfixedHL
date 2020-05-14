#ifndef MULTIMODE_CONFIG_VAR_H
#define MULTIMODE_CONFIG_VAR_H
#include <unordered_set>
#include <json.hpp>

enum class ModeID;

class MMConfigVarBase
{
public:
	virtual inline ~MMConfigVarBase() {}

	inline const char *GetName()
	{
		return m_Name;
	}

	/**
	 * Returns true if `json` can be stored in this config var.
	 */
	virtual inline bool ValidateValue(const nlohmann::json &json) = 0;

	/**
	 * Sets value by getting templated type from `json`.
	 */
	virtual inline void SetJSON(const nlohmann::json &json) = 0;

	/**
	 * Returns list of vars for a mode.
	 */
	static std::unordered_set<MMConfigVarBase *> &GetModeVars(ModeID mode);

protected:
	MMConfigVarBase() = default;
	MMConfigVarBase(const MMConfigVarBase &) = delete;

	const char *m_Name;
};

template <typename MODE, typename T>
class MMConfigVar : public MMConfigVarBase
{
public:
	inline MMConfigVar(const char *name, const T &defVal)
	{
		m_Name = name;
		m_Value = defVal;
		m_DefValue = defVal;
		GetModeVars(MODE::MODE_ID).insert(this);
	}

	inline virtual ~MMConfigVar()
	{
		GetModeVars(MODE::MODE_ID).erase(this);
	}

	/**
	 * Returns current value.
	 */
	inline const T &Get() const
	{
		return m_Value;
	}

	inline void Set(const T &val)
	{
		m_Value = val;
	}

	/**
	 * Returns true if `json` can be stored in this config var.
	 */
	virtual inline bool ValidateValue(const nlohmann::json &json) override
	{
		if (json.is_null())
			return true;

		try
		{
			json.get<T>();
			return true;
		}
		catch (nlohmann::json::type_error &)
		{
			return false;
		}
	}

	/**
	 * Sets value by getting templated type from `json`.
	 */
	virtual inline void SetJSON(const nlohmann::json &json) override
	{
		if (json.is_null())
			m_Value = m_DefValue;
		else
			m_Value = json.get<T>();
	}

private:
	T m_Value;
	T m_DefValue;
};

#endif
