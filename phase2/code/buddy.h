#include <string.h>
#include <math.h>

#define max_pair_size 512

 struct pair
{

    int start;
    int end;

};


 struct size_list
{

    struct pair pair_list[max_pair_size];
    int last_index;
};

struct buddy
{

   struct size_list lists[10];

};

// initialize lists list of 1024 of all memory 
void initialize_buddy(struct buddy *arr)
{
    // Initialize all indices to 0 (first lists index)
    for (int i = 0; i < 10; i++)
    {
        arr->lists[i].last_index = 0;
    }

    // Add block with size 1024 total memory 
    arr->lists[9].pair_list[arr->lists[9].last_index].start = 0;
    arr->lists[9].pair_list[arr->lists[9].last_index].end = 1023;
    arr->lists[9].last_index += 1;
}

struct pair allocate(struct buddy *arr , int allocation_size)
{
   
   struct pair result = {.start = -1, .end = -1};

    if (allocation_size == 0)
        return result;
     /*get the index of required size 
      index is from 0->9 */
    int n = ceil(log(allocation_size) / log(2)) - 1;
    if (n == -1)
        n = 0;

    if (arr->lists[n].last_index > 0)
    {

        
        result.start = arr->lists[n].pair_list[0].start;
        result.end = arr->lists[n].pair_list[0].end;

        
        arr->lists[n].pair_list[0] = arr->lists[n].pair_list[arr->lists[n].last_index - 1];
        arr->lists[n].last_index -= 1;
    }
    else
    {

        int i;
        
        for (i = n + 1; i < 10; i++)
        {
            if (arr->lists[i].last_index != 0)
                break;
        }

        
        if (i == 10)
        {
            return result;
        }

        
        result.start = arr->lists[i].pair_list[0].start;
        result.end = arr->lists[i].pair_list[0].end;

        
        arr->lists[i].pair_list[0] = arr->lists[i].pair_list[arr->lists[i].last_index - 1];
        arr->lists[i].last_index -= 1;

       
        i -= 1;
        while (i >= n)
        {

           struct pair pair1, pair2;
            pair1.start = result.start;
            pair1.end = result.start + (result.end - result.start) / 2;

            pair2.start = result.start + (result.end - result.start + 1) / 2;
            pair2.end = result.end;

            
            arr->lists[i].pair_list[arr->lists[i].last_index] = pair1;
            arr->lists[i].last_index += 1;

            arr->lists[i].pair_list[arr->lists[i].last_index] = pair2;
            arr->lists[i].last_index += 1;

            
            result.start = arr->lists[i].pair_list[0].start;
            result.end = arr->lists[i].pair_list[0].end;

            
            arr->lists[i].pair_list[0] = arr->lists[i].pair_list[arr->lists[i].last_index - 1];
            arr->lists[i].last_index -= 1;

            i -= 1;
        }
    }

   
    return result;
}

