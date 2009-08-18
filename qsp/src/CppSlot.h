#include "CppEvent.h"

template <typename ReturnT,typename ParamT>
class CppSlotBase1 
{
	public:
		
	virtual int connect(CppEvent1<ReturnT,ParamT>* cpp_event) {return 0;}
	virtual bool disconnect() {return false;}
};

template <typename ListenerT,typename ReturnT,typename ParamT>
class CppSlot1 : public CppSlotBase1<ReturnT,ParamT>
{

	public:
	
	typedef ReturnT (ListenerT::*PtrMember)(ParamT);

	CppSlot1(ListenerT* object, PtrMember member)
		: m_object(object), m_member(member), m_cpp_event(NULL)
	{}

	virtual ~CppSlot1()
	{
		//EgLOG(LogTRACE,"~CppSlot1 ...");

		if(m_cpp_event != NULL)
		{
			//EgLOG(LogTRACE,"~CppSlot1 detach ...");
			m_cpp_event->detach(m_handle);
		}

	}
	
	int connect(CppEvent1<ReturnT,ParamT>* cpp_event)
	{

		m_cpp_event = cpp_event;
		m_handle = m_cpp_event->attach(m_object,m_member);
		//EgLOG(LogTRACE,"CppSlot1::connect %d",m_handle);
			
		return m_handle;
	}
	
	bool disconnect()
	{
		//EgLOG(LogTRACE,"CppSlot1::disconnect() ... ");
		m_cpp_event = NULL;
		return true;
	}

	private:

	
	ListenerT* m_object;
	PtrMember m_member;

	CppEventHandler m_handle;
	CppEvent1<ReturnT,ParamT>* m_cpp_event;
};

template <typename ReturnT,typename ParamT>
class CppSignal1 : private CppEvent1<ReturnT,ParamT>
{

public:
	
	typedef typename std::map<int, CppSlotBase1<ReturnT,ParamT>* > SlotHandlersMap;
	typedef typename std::map<int, CppSlotBase1<ReturnT,ParamT>* >::iterator SlotHandlersMapIt;

	// disconnect all connected slots
	virtual ~CppSignal1()
	{
		//EgLOG(LogTRACE,"~CppSignal1() ...");
		SlotHandlersMapIt it = m_slots.begin();
		for(; it != m_slots.end(); it++)
		{
			if(isAttached(it->first))
			{
				//EgLOG(LogTRACE,"~CppSignal1 disconnect %d",it->first);
				it->second->disconnect();
			}
		}
	}

	// connect slot to the signal
	bool connect(CppSlotBase1<ReturnT,ParamT>* slot)
	{
		int handle = slot->connect(this);
		m_slots[handle] = slot;
		return true;
	}
	
	// emit signal 
	bool emit_sig(ParamT param)
	{
		notify(param);		
		return true;
	}

	template <typename CollectorT>
	typename CollectorT::return_type emit_sig(ParamT param,CollectorT& collect)
	{				
		return notify(param,collect);		
	}

private:

	SlotHandlersMap m_slots;
};

template <typename ReturnT,typename ParamT,typename ParamT1>
class CppSlotBase2 
{
	public:
		
	virtual int connect(CppEvent2<ReturnT,ParamT,ParamT1>* cpp_event) {return 0;}
	virtual bool disconnect() {return false;}
};

template <typename ListenerT,typename ReturnT,typename ParamT,typename ParamT1>
class CppSlot2 : public CppSlotBase2<ReturnT,ParamT,ParamT1>
{

	public:
	
	typedef ReturnT (ListenerT::*PtrMember)(ParamT,ParamT1);

	CppSlot2(ListenerT* object, PtrMember member)
		: m_object(object), m_member(member), m_cpp_event(NULL)
	{}

	virtual ~CppSlot2()
	{
		//EgLOG(LogTRACE,"~CppSlot1 ...");

		if(m_cpp_event != NULL)
		{
			//EgLOG(LogTRACE,"~CppSlot1 detach ...");
			m_cpp_event->detach(m_handle);
		}

	}
	
	virtual int connect(CppEvent2<ReturnT,ParamT,ParamT1>* cpp_event)
	{

		m_cpp_event = cpp_event;
		m_handle = m_cpp_event->attach(m_object,m_member);
		//EgLOG(LogTRACE,"CppSlot1::connect %d",m_handle);
			
		return m_handle;
	}
	
	virtual bool disconnect()
	{
		//EgLOG(LogTRACE,"CppSlot1::disconnect() ... ");
		m_cpp_event = NULL;
		return true;
	}

	private:

	
	ListenerT* m_object;
	PtrMember m_member;

	CppEventHandler m_handle;
	CppEvent2<ReturnT,ParamT,ParamT1>* m_cpp_event;
};

template <typename ReturnT,typename ParamT,typename ParamT1>
class CppSignal2 : private CppEvent2<ReturnT,ParamT,ParamT1>
{

public:
	
	typedef typename std::map<int, CppSlotBase2<ReturnT,ParamT,ParamT1>* > SlotHandlersMap;
	typedef typename std::map<int, CppSlotBase2<ReturnT,ParamT,ParamT1>* >::iterator SlotHandlersMapIt;

	// disconnect all connected slots
	virtual ~CppSignal2()
	{
		//EgLOG(LogTRACE,"~CppSignal1() ...");
		SlotHandlersMapIt it = m_slots.begin();
		for(; it != m_slots.end(); it++)
		{
			if(isAttached(it->first))
			{
				//EgLOG(LogTRACE,"~CppSignal1 disconnect %d",it->first);
				it->second->disconnect();
			}
		}
	}

	// connect slot to the signal
	bool connect(CppSlotBase2<ReturnT,ParamT,ParamT1>* slot)
	{
		int handle = slot->connect(this);
		m_slots[handle] = slot;
		return true;
	}
	
	// emit signal 
	bool emit_sig(ParamT p,ParamT1 p1)
	{
		notify(p,p1);		
		return true;
	}

	template <typename CollectorT>
	typename CollectorT::return_type emit_sig(ParamT p,ParamT1 p1,CollectorT& collect)
	{				
		return notify(p,p1,collect);		
	}

private:

	SlotHandlersMap m_slots;
};


template <typename ReturnT,typename ParamT,typename ParamT1,typename ParamT2>
class CppSlotBase3 
{
	public:
		
	virtual int connect(CppEvent3<ReturnT,ParamT,ParamT1,ParamT2>* cpp_event) {return 0;}
	virtual bool disconnect() {return false;}
};

template <typename ListenerT,typename ReturnT,typename ParamT,typename ParamT1,typename ParamT2>
class CppSlot3 : public CppSlotBase3<ReturnT,ParamT,ParamT1,ParamT2>
{

	public:
	
	typedef ReturnT (ListenerT::*PtrMember)(ParamT,ParamT1,ParamT2);

	CppSlot3(ListenerT* object, PtrMember member)
		: m_object(object), m_member(member), m_cpp_event(NULL)
	{}

	virtual ~CppSlot3()
	{
		//EgLOG(LogTRACE,"~CppSlot1 ...");

		if(m_cpp_event != NULL)
		{
			//EgLOG(LogTRACE,"~CppSlot1 detach ...");
			m_cpp_event->detach(m_handle);
		}
	}
	
	int connect(CppEvent3<ReturnT,ParamT,ParamT1,ParamT2>* cpp_event)
	{

		m_cpp_event = cpp_event;
		m_handle = m_cpp_event->attach(m_object,m_member);
		//EgLOG(LogTRACE,"CppSlot1::connect %d",m_handle);
			
		return m_handle;
	}
	
	bool disconnect()
	{
		//EgLOG(LogTRACE,"CppSlot1::disconnect() ... ");
		m_cpp_event = NULL;
		return true;
	}

	private:

	
	ListenerT* m_object;
	PtrMember m_member;

	CppEventHandler m_handle;
	CppEvent3<ReturnT,ParamT,ParamT1,ParamT2>* m_cpp_event;
};

template <typename ReturnT,typename ParamT,typename ParamT1,typename ParamT2>
class CppSignal3 : private CppEvent3<ReturnT,ParamT,ParamT1,ParamT2>
{

public:
	
	typedef typename std::map<int, CppSlotBase3<ReturnT,ParamT,ParamT1,ParamT2>* > SlotHandlersMap;
	typedef typename std::map<int, CppSlotBase3<ReturnT,ParamT,ParamT1,ParamT2>* >::iterator SlotHandlersMapIt;

	// disconnect all connected slots
	virtual ~CppSignal3()
	{
		//EgLOG(LogTRACE,"~CppSignal1() ...");
		SlotHandlersMapIt it = m_slots.begin();
		for(; it != m_slots.end(); it++)
		{
			if(isAttached(it->first))
			{
				//EgLOG(LogTRACE,"~CppSignal1 disconnect %d",it->first);
				it->second->disconnect();
			}
		}
	}

	// connect slot to the signal
	bool connect(CppSlotBase3<ReturnT,ParamT,ParamT1,ParamT2>* slot)
	{
		int handle = slot->connect(this);
		m_slots[handle] = slot;
		return true;
	}
	
	// emit signal 
	bool emit_sig(ParamT p,ParamT1 p1,ParamT2 p2)
	{
		notify(p,p1,p2);		
		return true;
	}

	template <typename CollectorT>
	typename CollectorT::return_type emit_sig(ParamT p,ParamT1 p1,ParamT2 p2,CollectorT& collect)
	{				
		return notify(p,p1,collect);		
	}

private:

	SlotHandlersMap m_slots;
};

