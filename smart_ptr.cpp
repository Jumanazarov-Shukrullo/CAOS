#include <utility> 
template<typename vt> 
class smart_ptr { 
public: 
    smart_ptr() noexcept(std::is_nothrow_default_constructible_v<vt>){ 
        ptr_ = new (std::nothrow) vt(); 
    }; 
  explicit smart_ptr(const size_t size) noexcept(std::is_nothrow_default_constructible_v<vt>): ptr_(new(std::nothrow) vt[size]) {} 
    ~smart_ptr() noexcept(std::is_nothrow_destructible_v<vt>){ 
     if (ptr_ != nullptr) { 
     ptr_->~vt(); 
        operator delete (ptr_, std::nothrow); 
        } 
    } 
    smart_ptr(const smart_ptr& other)= delete; 
    smart_ptr& operator=(const smart_ptr& other)= delete; 
    smart_ptr(smart_ptr&& other) noexcept : ptr_(other.ptr_){ 
     other.ptr_ = nullptr; 
    } 
    smart_ptr& operator= (smart_ptr&& other)& noexcept(std::is_nothrow_destructible_v<vt>){ 
        if (this != &other) { 
            this->~smart_ptr(); 
            new(this) smart_ptr(std::move(other)); 
        } 
        return *this; 
    } 
    vt& operator*() const noexcept{ 
        return *ptr_; 
    } 
    vt* operator->() const noexcept { 
        return ptr_; 
    } 
    vt* get() const noexcept { 
        return this->ptr_; 
    } 
private: 
    vt* ptr_; 
};
