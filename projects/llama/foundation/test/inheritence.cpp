
template <typename T> class TypeInfo
{
  public:
    
};

class Object
{
    template <typename T> friend class TypeInfo;

  public:
};

class Base1
{
};

class Base2
{
};

class Derived1
{
};