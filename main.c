#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#define CHUNK_CAP 640000
#define CHUNK_LIST_CAP 1024

typedef struct
{
  void *start;
  size_t size;
} Chunk;

typedef struct
{
  Chunk chunks[CHUNK_LIST_CAP];
  size_t size;
} Chunk_List;

void chunk_list_dump(Chunk_List *list, char name[])
{
  printf("Chunk List %s: size=%zu\n", name, list->size);
  for (size_t i = 0; i < list->size; i++)
  {
    Chunk chunk = list->chunks[i];
    printf("Chunk %zu: start=%p, size=%zu\n", i, chunk.start, chunk.size);
  }
}
void chunk_list_add(Chunk_List *list, void *start, size_t size)
{
  assert(list->size < CHUNK_LIST_CAP);
  list->chunks[list->size].start = start;
  list->chunks[list->size].size = size;

  for (size_t i = list->size;
       i > 0 && list->chunks[i].start < list->chunks[i - 1].start;
       i--)
  {
    Chunk tmp = list->chunks[i];
    list->chunks[i] = list->chunks[i - 1];
    list->chunks[i - 1] = tmp;
  }
  list->size++;
}

void chunk_list_merge(Chunk_List *dst, const Chunk_List *src)
{
  dst->size = 0;
  for (size_t i = 0; i < src->size; i++)
  {
    const Chunk chunk = src->chunks[i];
    if (dst->size > 0)
    {
      Chunk *last = &dst->chunks[dst->size - 1];
      if (last->start + last->size == chunk.start)
      {
        last->size += chunk.size;
      }
      else
      {
        chunk_list_add(dst, chunk.start, chunk.size);
      }
    }
    else
    {
      chunk_list_add(dst, chunk.start, chunk.size);
    }
  }
}

int chunk_list_find(const Chunk_List *list, void *ptr)
{
  for (size_t i = 0; i < list->size; ++i)
  {
    if (list->chunks[i].start == ptr)
    {
      return (int)i;
    }
  }
  return -1;
}

void chunk_list_remove(Chunk_List *list, size_t index)
{
  assert(index < list->size);
  for (size_t i = index; i < list->size - 1; i++)
  {
    list->chunks[i] = list->chunks[i + 1];
  }
  list->size -= 1;
}

char heap[CHUNK_CAP] = {0};

Chunk_List alloced_chunks = {0};
Chunk_List freed_chunks = {
    .chunks = {
        [0] = {
            .start = heap,
            .size = CHUNK_CAP,
        },
    },
    .size = 1,
};

Chunk_List tmp_chunks = {0};

void *heap_alloc(size_t size)
{
  if (size == 0)
  {
    return NULL;
  }
  chunk_list_merge(&tmp_chunks, &freed_chunks);
  freed_chunks = tmp_chunks;
  for (size_t i = 0; i < freed_chunks.size; i++)
  {
    Chunk chunk = freed_chunks.chunks[i];
    if (chunk.size >= size)
    {
      chunk_list_remove(&freed_chunks, i);
      const size_t tail = chunk.size - size;
      chunk_list_add(&alloced_chunks, chunk.start, size);
      if (tail > 0)
      {
        chunk_list_add(&freed_chunks, chunk.start + size, tail);
      }
      return chunk.start;
    }
  }
  return NULL;
}

void heap_free(void *ptr)
{
  if (ptr == NULL)
  {
    return;
  }
  const int index = chunk_list_find(&alloced_chunks, ptr);
  assert(index >= 0);

  chunk_list_add(&freed_chunks, alloced_chunks.chunks[index].start, alloced_chunks.chunks[index].size);
  chunk_list_remove(&alloced_chunks, (size_t) index);
  return NULL;
}

void heap_collect()
{
  return NULL;
}

#define N 10

void *ptrs[N] = {0};

int main()
{
  for (int i = 0; i < N; i++)
  {
    ptrs[i] = heap_alloc(i);
  }

  for (int i = 0; i < 10; i++)
  {
    if (i % 2 == 0)
    {
      heap_free(ptrs[i]);
    }
  }

  heap_alloc(10);
  heap_alloc(2);
  chunk_list_dump(&alloced_chunks, "Alloced");
  chunk_list_dump(&freed_chunks, "Freed");
  printf("Sucessful\n");
  return 0;
}