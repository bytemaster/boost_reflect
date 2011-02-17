
class describe_visitor
{
    public:
    describe_visitor():depth(0){}
   int depth;


    friend inline void visit( const signed_int& str, describe_visitor& v, uint32_t f = -1 )
    {
        std::cerr<<std::setw(4*v.depth)<<" "<<str.value<<std::endl;
    }
    friend inline void visit( const unsigned_int& str, describe_visitor& v, uint32_t f = -1 )
    {
        std::cerr<<std::setw(4*v.depth)<<" "<<str.value<<std::endl;
    }
    friend inline void visit( const std::string& str, describe_visitor& v, uint32_t f = -1 )
    { 
        std::cerr<<std::setw(4*v.depth)<<" "<<'"'<<str<<'"'<<std::endl;
    }


   template<typename Class, typename T, typename Flags>
   void accept_member( Class& c, T (Class::*p), const char* name, Flags f )
   {
        std::cerr<<std::setw(4*depth)<<" "<< boost::idl::get_typename<T>()<<" "<<boost::idl::get_typename<Class>()<<"::"<<name<< " = {\n";
        ++depth;
        visit( c.*p, *this );
        --depth;
        std::cerr<<std::setw(4*depth)<<" "<<"}\n";
   }
    template<typename Class, typename Field, typename Alloc, template<typename,typename> class Container, typename Flags>
    void accept_member( Class c, Container<Field,Alloc> (Class::*p), const char* name, Flags key )
    {
        std::cerr<<std::setw(4*depth)<<" "<< " "<<boost::idl::get_typename<Class>()<<"::"<<name<< " = [\n";
        typename Container<Field,Alloc>::const_iterator itr = (c.*p).begin();
        typename Container<Field,Alloc>::const_iterator end = (c.*p).end();
        ++depth;
        while( itr != end )
        {
            std::cerr<<std::setw(4*depth)<<" " << "{\n";
            ++depth;
            visit( *itr, *this );
            --depth;
            std::cerr<<std::setw(4*depth)<<" "<<"}\n";
            ++itr;
        }
        --depth;
        std::cerr<<std::setw(4*depth)<<" "<<"]\n";
    }
   template<typename Class, typename T, typename Flags>
   void accept_member( const Class& c, T (Class::*p), const char* name, Flags f )
   {
        std::cerr<<std::setw(4*depth)<<" "<< boost::idl::get_typename<T>()<<" "<<boost::idl::get_typename<Class>()<<"::"<<name<< " = {\n";
        ++depth;
        visit( c.*p, *this );
        --depth;
        std::cerr<<std::setw(4*depth)<<" "<<"}\n";
   }
    template<typename Class, typename Flags>
    void accept_member( Class c, signed_int (Class::*p), const char* name, Flags key )
    {
        std::cerr<<std::setw(4*depth)<<" "<< boost::idl::get_typename<int>()<<" "<<boost::idl::get_typename<Class>()<<"::"<<name<< " = " << (c.*p).value << std::endl;
    }
    
    template<typename Class, typename Flags>
    void accept_member( Class c, unsigned_int (Class::*p), const char* name, Flags key )
    {
        std::cerr<<std::setw(4*depth)<<" "<< boost::idl::get_typename<int>()<<" "<<boost::idl::get_typename<Class>()<<"::"<<name<< " = " << (c.*p).value << std::endl;
    }
    
   template<typename Class, typename Flags>
   void accept_member( Class c, int (Class::*p), const char* name, Flags f )
   {
        c.*p += 2;
        std::cerr<<std::setw(4*depth)<<" "<< boost::idl::get_typename<int>()<<" "<<boost::idl::get_typename<Class>()<<"::"<<name<< " = " << c.*p << std::endl;
   }
   template<typename Class, typename Flags>
   void accept_member( const Class& c, int (Class::*p), const char* name, Flags f )
   {
        std::cerr<<std::setw(4*depth)<<" "<< boost::idl::get_typename<int>()<<" "<<boost::idl::get_typename<Class>()<<"::"<<name<< " = " << c.*p << std::endl;
   }
   template<typename Class, typename Flags>
   void accept_member( Class c, std::string (Class::*p), const char* name, Flags f )
   {
        std::cerr<<std::setw(4*depth)<<" "<< boost::idl::get_typename<std::string>()<<" "<<boost::idl::get_typename<Class>()<<"::"<<name<< " = \"" << c.*p << "\"\n";
   }
   template<typename Class,typename Flags>
   void accept_member( Class c, double (Class::*p), const char* name, Flags f )
   {
        std::cerr<<std::setw(4*depth)<<" "<< boost::idl::get_typename<double>()<<" "<<boost::idl::get_typename<Class>()<<"::"<<name<< " = " << c.*p << std::endl;
   }

   template<typename Class,typename Flags>
   void accept_member( Class c, float (Class::*p), const char* name, Flags f )
   {
        std::cerr<<std::setw(4*depth)<<" "<< boost::idl::get_typename<float>()<<" "<<boost::idl::get_typename<Class>()<<"::"<<name<< " = " << c.*p << std::endl;
   }

   template<typename Class>
    void not_found( Class c, uint32_t field )
    {
        std::cerr<<"NOT FOUND FIELD "<< field <<std::endl;
    }
};
