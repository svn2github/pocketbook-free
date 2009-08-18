#ifndef CPP_EVENT_HEADER
#define CPP_EVENT_HEADER

#include <map>
#include <vector>

//#include <EgLogger.h>

typedef  int CppEventHandler;



template <typename ReturnT,typename ParamT>
class EventHandlerBase1
{
public:

	virtual ReturnT notify(ParamT param) = 0;

};

template <typename ListenerT,typename ReturnT,typename ParamT>
class EventHandler1 : public EventHandlerBase1<ReturnT,ParamT>
{
	
	typedef ReturnT (ListenerT::*PtrMember)(ParamT);
	ListenerT* m_object;
	PtrMember m_member;

public:


	EventHandler1(ListenerT* object, PtrMember member)
		: m_object(object), m_member(member)
	{}

	ReturnT notify(ParamT param)
	{
		return (m_object->*m_member)(param);		
	}	

};

template <typename ReturnT,typename ParamT>
class CppEvent1
{
	typedef typename std::map<int,EventHandlerBase1<ReturnT,ParamT> *> HandlersMap;
	typedef typename std::map<int,EventHandlerBase1<ReturnT,ParamT> *>::iterator HandlersMapIt;
	HandlersMap m_handlers;
	int m_count;

	public:

	CppEvent1()
		: m_count(0) {}

	virtual ~CppEvent1() 
	{
		// delete all event handlers
		for(HandlersMapIt it = m_handlers.begin(); it != m_handlers.end(); ++it)
			delete it->second;
	}

	template <typename ListenerT>
	CppEventHandler attach(ListenerT* object,ReturnT (ListenerT::*member)(ParamT))
	{
		typedef ReturnT (ListenerT::*PtrMember)(ParamT);	
		m_handlers[m_count] = (new EventHandler1<ListenerT,ReturnT,ParamT>(object,member));
		m_count++;	
		return m_count-1;
	}

	bool detach(CppEventHandler id)
	{
		//EgLOG(LogTRACE,"CppEvent1::detach %d",id);
		HandlersMapIt it = m_handlers.find(id);

		if(it == m_handlers.end())
			return false;
		
//		EgLOG(LogTRACE,"CppEvent1::detach done ...");
		delete it->second;
		m_handlers.erase(it);				
		return true;
	}

	virtual bool isAttached(CppEventHandler id)
	{
		return m_handlers.find(id) != m_handlers.end();
	}

	void notify(ParamT param)
	{
		HandlersMapIt it = m_handlers.begin();		
		for(; it != m_handlers.end(); it++)
		{			
			it->second->notify(param);
		}
	}

	template <typename CollectorT>
	typename CollectorT::return_type notify(ParamT param,CollectorT& collect)
	{
		HandlersMapIt it = m_handlers.begin();
		std::vector<ReturnT> results;
		for(; it != m_handlers.end(); it++)
		{			
			results.push_back(it->second->notify(param));
		}

		return collect(results.begin(),results.end());
	}
};


// do the same for delegates with 2 parameters
template <typename ReturnT,typename ParamT,typename ParamT1>
class EventHandlerBase2
{
public:
	virtual ReturnT notify(ParamT param,ParamT1 param1) = 0;
};

template <typename ListenerT,typename ReturnT,typename ParamT,typename ParamT1>
class EventHandler2 : public EventHandlerBase2<ReturnT, ParamT, ParamT1>
{
	typedef ReturnT (ListenerT::*PtrMember)(ParamT,ParamT1);
	ListenerT* m_object;
	PtrMember m_member;
	
public:

	EventHandler2(ListenerT* object, PtrMember member)
		: m_object(object), m_member(member)
	{}

	ReturnT notify(ParamT param,ParamT1 param1)
	{
		return (m_object->*m_member)(param,param1);		
	}	
};

template <typename ReturnT,typename ParamT,typename ParamT1>
class CppEvent2
{
	typedef typename std::map<int,EventHandlerBase2<ReturnT,ParamT,ParamT1> *> HandlersMap;
	typedef typename std::map<int,EventHandlerBase2<ReturnT,ParamT,ParamT1> *>::iterator HandlersMapIt;
	
	HandlersMap m_handlers;
	int m_count;

public:

	
	CppEvent2()
		: m_count(0) {}

	virtual ~CppEvent2() {}
	
	template <typename ListenerT>
	CppEventHandler attach(ListenerT* object,ReturnT (ListenerT::*member)(ParamT,ParamT1))
	{
		typedef ReturnT (ListenerT::*PtrMember)(ParamT,ParamT1);	
		m_handlers[m_count] = (new EventHandler2<ListenerT,ReturnT,ParamT,ParamT1>(object,member));
		m_count++;	
		return m_count-1;
	}

	bool detach(CppEventHandler id)
	{
		HandlersMapIt it = m_handlers.find(id);

		if(it == m_handlers.end())
			return false;
		
		delete it->second;
		m_handlers.erase(it);
		return true;
	}

	void notify(ParamT param,ParamT1 param1)
	{
		HandlersMapIt it = m_handlers.begin();
		for(; it != m_handlers.end(); it++)
		{
			it->second->notify(param,param1);
		}
	}
	
	virtual bool isAttached(CppEventHandler id)
	{
		return m_handlers.find(id) != m_handlers.end();
	}

	template <typename CollectorT>
	typename CollectorT::return_type notify(ParamT param,ParamT1 param1,CollectorT& collect)
	{
		HandlersMapIt it = m_handlers.begin();
		std::vector<ReturnT> results;
		for(; it != m_handlers.end(); it++)
		{			
			results.push_back(it->second->notify(param,param1));
		}

		return collect(results.begin(),results.end());
	}
};

// do the same for delegates with 3 parameters
template <typename ReturnT,typename ParamT,typename ParamT1,typename ParamT2>
class EventHandlerBase3
{
public:
	virtual ReturnT notify(ParamT param,ParamT1 param1,ParamT2 param2) = 0;
};

template <typename ListenerT,typename ReturnT,typename ParamT,typename ParamT1,typename ParamT2>
class EventHandler3 : public EventHandlerBase3<ReturnT,ParamT,ParamT1,ParamT2>
{
	typedef ReturnT (ListenerT::*PtrMember)(ParamT,ParamT1,ParamT2);
	ListenerT* m_object;
	PtrMember m_member;
	
public:

	EventHandler3(ListenerT* object, PtrMember member)
		: m_object(object), m_member(member)
	{}

	ReturnT notify(ParamT param,ParamT1 param1,ParamT2 param2)
	{
		return (m_object->*m_member)(param,param1,param2);		
	}	
};

template <typename ReturnT,typename ParamT,typename ParamT1,typename ParamT2>
class CppEvent3
{
	typedef typename std::map<int,EventHandlerBase3<ReturnT,ParamT,ParamT1,ParamT2> *> HandlersMap;
	typedef typename std::map<int,EventHandlerBase3<ReturnT,ParamT,ParamT1,ParamT2> *>::iterator HandlersMapIt;
	
	HandlersMap m_handlers;
	int m_count;

public:

	
	CppEvent3()
		: m_count(0) {}

	template <typename ListenerT>
	CppEventHandler attach(ListenerT* object,ReturnT (ListenerT::*member)(ParamT,ParamT1,ParamT2))
	{
		typedef ReturnT (ListenerT::*PtrMember)(ParamT,ParamT1,ParamT2);	
		m_handlers[m_count] = (new EventHandler3<ListenerT,ReturnT,ParamT,ParamT1,ParamT2>(object,member));
		m_count++;	
		return m_count-1;
	}

	bool detach(CppEventHandler id)
	{
		HandlersMapIt it = m_handlers.find(id);

		if(it == m_handlers.end())
			return false;
		
		delete it->second;
		m_handlers.erase(it);
		return true;
	}

	void notify(ParamT param,ParamT1 param1,ParamT2 param2)
	{
		HandlersMapIt it = m_handlers.begin();
		for(; it != m_handlers.end(); it++)
		{
			it->second->notify(param,param1,param2);
		}
	}
	
	virtual bool isAttached(CppEventHandler id)
	{
		return m_handlers.find(id) != m_handlers.end();
	}

	template <typename CollectorT>
	typename CollectorT::return_type notify(ParamT param,ParamT1 param1,ParamT2 param2,CollectorT& collect)
	{
		HandlersMapIt it = m_handlers.begin();
		std::vector<ReturnT> results;
		for(; it != m_handlers.end(); it++)
		{			
			results.push_back(it->second->notify(param,param1,param2));
		}

		return collect(results.begin(),results.end());
	}
};

#endif
