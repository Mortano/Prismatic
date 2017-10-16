//----Global new/delete----

void* operator new(size_t size)
{
	return GlobalAllocator->Allocate(size);
}

void* operator new[](size_t size)
{
	return GlobalAllocator->Allocate(size);
}

void operator delete(void* mem)
{
	GlobalAllocator->Free(mem);
}

void operator delete[](void* mem)
{
	GlobalAllocator->Free(mem);
}