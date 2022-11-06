#pragma once
#include "webserv.hpp"

class JSONNode {
  public:
	JSONObject obj();
	JSONList lst();
	string str();
	int nbr();
	bool boo();

  private:
	enum Type { OBJECT, LIST, STRING, NUMBER, BOOLEAN, NULL_TYPE };
	union Values {
		JSONObject *object;
		JSONList *list;
		string *str;
		int nbr;
		bool bValue;
	} values;
	Type type;

	void setObject(JSONObject *object);
	void setList(JSONList *list);
	void setString(string *str);
	void setNumber(int nbr);
	void setBoolean(bool v);
	void setNull();

	string stringify(int indentationLevel);
	friend class JSONParser;
};
