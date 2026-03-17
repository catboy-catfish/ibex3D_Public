#include <ibex3D/utility/memoryFunctions.h>

struct myStruct_t
{
	int ID = 0;

	myStruct_t()
	{
		printf("Constructor called for myStruct with ID %d.\n", ID);
	}

	myStruct_t(int inputID)
	{
		ID = inputID;
		printf("Constructor called for myStruct with ID %d.\n", ID);
	}

	~myStruct_t()
	{
		printf("Destructor called for myStruct with ID %d.\n", ID);
	}
};

int main()
{	
	myStruct_t* myStruct = ibex3D_alloc<myStruct_t>();
	myStruct_t* myStruct69 = ibex3D_alloc<myStruct_t>(69);

	system("pause");

	ibex3D_free<myStruct_t>(myStruct69);
	myStruct69 = nullptr;

	ibex3D_free<myStruct_t>(myStruct);
	myStruct = nullptr;

	system("pause");
	return 0;
}