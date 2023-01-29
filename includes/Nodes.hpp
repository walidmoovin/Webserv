#pragma once
#include "webserv.hpp"

class JSONNode {
public:
	JSONObject	obj(void);
	JSONList	lst(void);
	string	str(void);
	int	nbr(void);
	bool	boo(void);
	~JSONNode(void);

private:
	enum Type { OBJECT, LIST, STRING, NUMBER, BOOLEAN, NULL_TYPE };
	union Values {
		JSONObject	*object;
		JSONList	*list;
		string	*str;
		int	nbr;
		bool	bValue;
	} values;
	Type type;

	void	setObj(JSONObject *object);
	void	setLst(JSONList *list);
	void	setStr(string *str);
	void	setNbr(int nbr);
	void	setBoo(bool v);
	void	setNull(void);

	string	stringify(int indentationLevel);
	friend	class JSONParser;
};
