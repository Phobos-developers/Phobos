#pragma once

// Source code from SEARCH.H in CncRemastered

template<class T>
class IndexClass
{
public:
	IndexClass(void);
	~IndexClass(void);

	bool Add_Index(int id, T data);
	bool Remove_Index(int id);
	bool Is_Present(int id) const;
	int Count(void) const;
	T Fetch_Index(int id) const;
	void Clear(void);

private:
	struct NodeElement {
		int ID;
		T Data;
	};

	NodeElement* IndexTable;
	int IndexCount;
	int IndexSize;
	bool IsSorted;
	NodeElement const* Archive;
	IndexClass(IndexClass const& rvalue);
	// IndexClass* operator = (IndexClass const& rvalue);

	bool Increase_Table_Size(int amount);
	bool Is_Archive_Same(int id) const;
	void Invalidate_Archive(void);
	void Set_Archive(NodeElement const* node);
	NodeElement const* Search_For_Node(int id) const;
	static int _USERENTRY search_compfunc(void const* ptr, void const* ptr2);
};

template<class T>
IndexClass<T>::IndexClass(void) :
	IndexTable(0),
	IndexCount(0),
	IndexSize(0),
	IsSorted(false),
	Archive(0)
{
	Invalidate_Archive();
}

template<class T>
IndexClass<T>::~IndexClass(void)
{
	Clear();
}

template<class T>
void IndexClass<T>::Clear(void)
{
	delete[] IndexTable;
	IndexTable = 0;
	IndexCount = 0;
	IndexSize = 0;
	IsSorted = false;
	Invalidate_Archive();
}

template<class T>
bool IndexClass<T>::Increase_Table_Size(int amount)
{
	if (amount < 0) return(false);

	NodeElement* table = new NodeElement[IndexSize + amount];
	if (table != NULL) {
		for (int index = 0; index < IndexCount; index++) {
			table[index] = IndexTable[index];
		}
		delete[] IndexTable;
		IndexTable = table;
		IndexSize += amount;
		Invalidate_Archive();

		return(true);
	}
	return(false);
}

template<class T>
int IndexClass<T>::Count(void) const
{
	return(IndexCount);
}

template<class T>
bool IndexClass<T>::Is_Present(int id) const
{
	if (IndexCount == 0) {
		return(false);
	}

	if (Is_Archive_Same(id)) {
		return(true);
	}

	NodeElement const* nodeptr = Search_For_Node(id);

	if (nodeptr != 0) {
		((IndexClass<T>*)this)->Set_Archive(nodeptr);
		return(true);
	}

	return(false);
}

template<class T>
T IndexClass<T>::Fetch_Index(int id) const
{
	if (Is_Present(id)) {
		return(Archive->Data);
	}
	return(T());
}

template<class T>
bool IndexClass<T>::Is_Archive_Same(int id) const
{
	if (Archive != 0 && Archive->ID == id) {
		return(true);
	}
	return(false);
}

template<class T>
void IndexClass<T>::Invalidate_Archive(void)
{
	Archive = 0;
}

template<class T>
void IndexClass<T>::Set_Archive(NodeElement const* node)
{
	Archive = node;
}

template<class T>
bool IndexClass<T>::Add_Index(int id, T data)
{
	if (IndexCount + 1 > IndexSize) {
		if (!Increase_Table_Size(IndexSize == 0 ? 10 : IndexSize)) {
			return(false);
		}
	}

	IndexTable[IndexCount].ID = id;
	IndexTable[IndexCount].Data = data;
	IndexCount++;
	IsSorted = false;

	return(true);
}

template<class T>
bool IndexClass<T>::Remove_Index(int id)
{
	int found_index = -1;
	for (int index = 0; index < IndexCount; index++) {
		if (IndexTable[index].ID == id) {
			found_index = index;
			break;
		}
	}

	if (found_index != -1) {

		for (int index = found_index + 1; index < IndexCount; index++) {
			IndexTable[index - 1] = IndexTable[index];
		}
		IndexCount--;

		NodeElement fake;
		fake.ID = 0;
		fake.Data = T();
		IndexTable[IndexCount] = fake;

		Invalidate_Archive();
		return(true);
	}

	return(false);
}

//template<class T>
//int IndexClass<T>::search_compfunc(void const* ptr1, void const* ptr2)
//{
//	if (*(int const*)ptr1 == *(int const*)ptr2) {
//		return(0);
//	}
//	if (*(int const*)ptr1 < *(int const*)ptr2) {
//		return(-1);
//	}
//	return(1);
//}

template<class T>
typename IndexClass<T>::NodeElement const* IndexClass<T>::Search_For_Node(int id) const
{
	if (IndexCount == 0) {
		return(0);
	}
	if (!IsSorted) {
		qsort(&IndexTable[0], IndexCount, sizeof(IndexTable[0]), search_compfunc);
		((IndexClass<T>*)this)->Invalidate_Archive();
		((IndexClass<T>*)this)->IsSorted = true;
	}
	NodeElement node;
	node.ID = id;
	return((NodeElement const*)bsearch(&node, &IndexTable[0], IndexCount, sizeof(IndexTable[0]), search_compfunc));
}