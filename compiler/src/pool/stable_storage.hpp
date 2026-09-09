#pragma once


#include <cassert>
#include <cstddef>
#include <memory_resource>
#include <ranges>
#include <utility>
#include <vector>

class StableStorage final
{
public:
  StableStorage() = default;

  StableStorage(const StableStorage&)            = delete;
  StableStorage& operator=(const StableStorage&) = delete;

  StableStorage(StableStorage&&)            = delete;
  StableStorage& operator=(StableStorage&&) = delete;

  ~StableStorage() noexcept
  {
    clear();
  }

  template <typename T, typename... Args>
  [[nodiscard]]
  size_t create(Args&&... args)
  {
    void* mem = resource.allocate(sizeof(T), alignof(T));

    T* obj = std::construct_at(static_cast<T*>(mem), std::forward<Args>(args)...);

    objects.push_back({obj, [](void* ptr) noexcept { std::destroy_at(static_cast<T*>(ptr)); }});

    return objects.size() - 1;
  }


  template <typename T, typename... Args>
  [[nodiscard]]
  T* create_get(Args&&... args)
  {
    void* mem = resource.allocate(sizeof(T), alignof(T));

    T* obj = std::construct_at(static_cast<T*>(mem), std::forward<Args>(args)...);

    objects.push_back({obj, [](void* ptr) noexcept { std::destroy_at(static_cast<T*>(ptr)); }});

    return obj;
  }

  template <typename T>
  [[nodiscard]]
  T* get(size_t index) noexcept
  {
    assert(index < objects.size());
    return static_cast<T*>(objects[index].ptr);
  }

  template <typename T>
  [[nodiscard]]
  const T* get(size_t index) const noexcept
  {
    assert(index < objects.size());
    return static_cast<const T*>(objects[index].ptr);
  }

  [[nodiscard]]
  size_t size() const noexcept
  {
    return objects.size();
  }

  void clear() noexcept
  {
    for (auto& object : std::ranges::reverse_view(objects)) object.destroy(object.ptr);

    objects.clear();
    resource.release();
  }

private:
  struct Destructor final {
    void* ptr;

    void (*destroy)(void*) noexcept;
  };

  std::pmr::monotonic_buffer_resource resource;
  std::vector<Destructor>             objects;
};