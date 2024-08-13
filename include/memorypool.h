#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H
#include <functional>
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#define M_POOL_DEFAULT_SIZE (16 * 1024)
#define M_MAX_ALLOC_FROM_POOL (4096 - 1)
#define M_ALIGNMENT 16

using m_pool_cleanup_pt = std::function<bool(void *)>;
//*自定义的清理结构体
struct m_pool_cleanup_s {
  m_pool_cleanup_pt handler;
  void *data;
  m_pool_cleanup_s *next;
};

//*大块内存
struct m_pool_large_s {
  m_pool_large_s *next;
  void *alloc;
};

struct m_pool_t;
//*小块内存
typedef struct {
  u_char *last;
  u_char *end;
  m_pool_t *next;
  uint failed;
} m_pool_data_t;
//*内存池主结构
struct m_pool_t {
  m_pool_data_t d;
  size_t max;
  m_pool_t *current;
  m_pool_large_s *large;
  m_pool_cleanup_s *cleanup;
};

class MemoryPool {
public:
  MemoryPool(int size);
  bool createPool(size_t size);
  bool destoryPool();
  bool resetPool();
  void *palloc(size_t size);
  void *pnalloc(size_t size);
  void *pcalloc(std::size_t size);
  void *memalign(size_t size, size_t alignment);
  int free(void *p);
  void cleanupAdd(m_pool_cleanup_pt cb, size_t size);

  ~MemoryPool();

private:
  m_pool_t *m_pool;
  int m_size;
};

#endif // !MEMORYPOOL_H
