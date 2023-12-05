#include "cstack.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

struct StackNode // Элемент стека
{
    struct StackNode* prev;
    void* data;
    unsigned int nodeSize;
};

typedef struct StackNode StackNode_t;

struct StackInstance // Экземпляр стека (реализация схожа с двусвязным списком)
{
    hstack_t hstack;
    struct StackInstance* prev;
    struct StackInstance* next;
    unsigned int stackSize;
    StackNode_t* stackPtr;
};

typedef struct StackInstance StackInstance_t;

StackInstance_t* FindStack(hstack_t hstack); // Функция для поиска экземпляра стека по его хэндлеру
void FreeStackInstance(StackInstance_t* stackInstance); // Функция для освобождения памяти, выделенной для экземпляра стека

StackInstance_t* lastStackInstance;
unsigned int stackCount = 0u; // Количество экземляров стека
hstack_t stackIdx = 0; // Порядковый номер (хэндлер) для создаваемого экземпляра стека

hstack_t stack_new(void)
{
    // Выделение памяти для нового экземпляра стека
    StackInstance_t* newStackInstance = (StackInstance_t*)malloc(sizeof(StackInstance_t));
    newStackInstance->next = NULL;
    newStackInstance->hstack = stackIdx;
    newStackInstance->stackSize = 0u;
    newStackInstance->stackPtr = NULL;
    if (stackCount == 0u)
    {
        newStackInstance->prev = NULL;
    }
    else
    {
        lastStackInstance->next = newStackInstance;
        newStackInstance->prev = lastStackInstance;
    }
    lastStackInstance = newStackInstance;
    ++stackCount;
    return stackIdx++;
}

void stack_free(const hstack_t hstack)
{
    StackInstance_t* stackToFree = FindStack(hstack);
    if (stackToFree != NULL)
    {
        // Если экземпляр стека найден, то память, выделенная для него, освобождается
        FreeStackInstance(stackToFree);
        --stackCount;
    }    
}

int stack_valid_handler(const hstack_t hstack)
{
    if (FindStack(hstack) != NULL)
    {
        return 0;
    }    
    return 1;
}

unsigned int stack_size(const hstack_t hstack)
{
    StackInstance_t* stack = FindStack(hstack);
    if (stack != NULL)
    {
        return stack->stackSize;
    }
    return 0;
}

void stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    StackInstance_t* stack = FindStack(hstack);
    if (stack != NULL && data_in != NULL && size != 0u)
    {
        // Выделение памяти для данных
        void* data = (void*)malloc(size);
        memcpy (data, data_in, size);
        // Выделение памяти для элемента стека
        StackNode_t* newStackNode = (StackNode_t*)malloc(sizeof(StackNode_t));
        StackNode_t tmpStackNode = { stack->stackPtr, data, size };
        memcpy(newStackNode, &tmpStackNode, sizeof(StackNode_t));
        stack->stackPtr = newStackNode;
        ++(stack->stackSize);
    }
}

unsigned int stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    StackInstance_t* stack = FindStack(hstack);
    if (stack != NULL && stack->stackPtr != NULL && data_out != NULL && size != 0u)
    {
        unsigned int nodeSize = stack->stackPtr->nodeSize;
        if (nodeSize <= size) // Проверка достаточности длины выделенного буфера
        {
            memcpy(data_out, stack->stackPtr->data, nodeSize);
            // Освобождение памяти, выделенной для данных последнего элемента стека
            free(stack->stackPtr->data);
            StackNode_t* prevNode = stack->stackPtr->prev;
            // Освобождение памяти, выделенной для последнего элемента стека
            free(stack->stackPtr);
            stack->stackPtr = prevNode;
            --(stack->stackSize);
            return nodeSize;
        }
    }
    return 0;
}

StackInstance_t* FindStack(hstack_t hstack)
{
    if (stackCount != 0u && hstack >= 0)
    {
        StackInstance_t* tmpStackInstance = lastStackInstance;
        do
        {
            if (tmpStackInstance->hstack == hstack)
            {
                return tmpStackInstance;
            }
            tmpStackInstance = tmpStackInstance->prev;
        }while (tmpStackInstance != NULL);
    }
    return NULL;
}

void FreeStackInstance(StackInstance_t* stack)
{
    // Если стек содержит элементы данных, то память, выделенная для них, освобождается
    if (stack->stackPtr != NULL)
    {
        do
        {
            free(stack->stackPtr->data);
            StackNode_t* prevNode = stack->stackPtr->prev;
            free(stack->stackPtr);
            stack->stackPtr = prevNode;
        } while (stack->stackPtr != NULL);
        free(stack->stackPtr);
    }
    stack->stackSize = 0u;
    stack->hstack = -1;
    if (stack->next != NULL)
    {
        stack->next->prev = stack->prev;
    }
    if (stack->prev != NULL)
    {
        stack->prev->next = stack->next;
    }
    // Освобождение памяти, выделенной для экземпляра стека
    free(stack);
}