/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a>
  <PRE> If that doesn't help, contact Jonathan Clark at
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

/// \file
/// String Classes
///
///   Strings are hidden behind the two classes :
///   \par
///       i4_const_str   (constant strings.. cannot be changed)
///   \par
///   and i4_str         (modifiable strings)
///
///   \par
///   i4_strs can only be created from i4_const_strs or other i4_strs and i4_const_strs can
///   only be created through the i4_string_manager.  All strings gotten from i4_string_manager
///   are loaded from an external file.  This ensures that all strings are stored external and
///   UNICODE or wide-chars can be added in without any code changes outside this module.
///   Because for most applications only one string manager is needed I have created a global
///   one called i4_string_man.  
///
///   \par
///   To see if a string is available from the string manager the call
///
///   \code const i4_const_str *s=&i4_string_man.get(str); \endcode
///
///   is made.  If s->null()==i4_T then the string was not loaded.
///   Since this is a common operation the short-named function i4gets is provided 
///   (see end of this file).
///
///
///   example :
///   \code
///   resource.res
///   ---------------
///   hello "Hello World"
///
///   ---------------
///
///
///   i4gets("hello") ->  i4_const_str("Hello World")
///
///   \endcode
///   \par
///   So, you might ask, how do I create a string quick and dirty for debugging purposes?
///
///   You have to do it the hard way for now.......

#ifndef __STRING_HPP_
#define __STRING_HPP_

#include "arch.h"
#include "init/init.h"
#include "error/error.h"
#include "memory/malloc.h"

#include <stdlib.h>
#include <string.h>
#undef new

/** This class represents a single character.

  It may seem overkill to have an entire class for a single character,
  but this class is merely used when really only a single character is
  interesting and not for collections of characters.
  Use it together with some of the functions of i4_const_str.
*/
class i4_char
{
protected:
  w8 ch; ///< The only data element of this class.
public:
  /// The copy constructor.
  i4_char(w8 ch) : ch(ch) {}
  /// The default constructor.
  i4_char():ch(0){}
  /// Returns the value of ch. 
  /// This return value is w16 because this function is subject
  /// to be expanded to double-byte charater sets sometime.
  w16 value() const { return ch; }
  /// Returns true if we are talking about a space character.
  /// A space character is ' ', '\\r', '\\t' or '\\n'.
  i4_bool is_space() const { return (i4_bool)(ch==' ' || ch=='\r' || ch=='\n' || ch=='\t'); }
  i4_bool is_slash() const { return (i4_bool)(ch=='/'); }
  i4_bool is_backslash() const { return (i4_bool)(ch=='\\'); }
  i4_bool is_period() const { return (i4_bool)(ch=='.'); }
#ifndef _WINDOWS
  
  i4_char to_lower() const 
	  { return (ch>='A' && ch<='Z') ? i4_char(ch-'A'+'a') : i4_char(ch); }
  i4_char to_upper() const 
  { return (ch>='a' && ch<='z') ? i4_char(ch-'a'+'A') : i4_char(ch); }
#else
  /// Converts one character to lowercase.
  /// For Windows, this uses the system-locale specific conversation function,
  /// which guarantees that language specific letters (i.e äöñ) are handled
  /// properly. 
  i4_char to_lower() const;
  /// Same thing, opposite direction.
  /// @see to_lower()
  i4_char to_upper() const;
#endif
  i4_bool operator==(const i4_char &b) const { return (ch == b.ch); }
  i4_bool operator!=(const i4_char &b) const { return (ch != b.ch); }
  i4_bool operator>(const i4_char &b) const { return  (ch > b.ch); }
  i4_bool operator<(const i4_char &b) const { return  (ch < b.ch); }
  i4_bool operator>=(const i4_char &b) const { return (ch >= b.ch); }
  i4_bool operator<=(const i4_char &b) const { return (ch <= b.ch); }
  // this should only be called if you know the string you are looking at has ascii chars in it
  w8 ascii_value() const { return ch; }
  
};


class i4_str;
class i4_string_manager_class;
class i4_file_class;

/** The base class of all string classes.
  This class can handle constants strings that do not change.
  Warning: The pointer to the string is \a not copied when the constructor
  is called, so be sure that the string which is represented by this class
  is not deleted before the class itself!
*/
class i4_const_str
{
protected:
  typedef char char_type;///< The base type of the string elements
  char_type *ptr; ///< The pointer to the string, 0 if empty.
  w16   len; ///< The length of the string (excluding the terminal 0, if any)

  friend class i4_string_manager_class;

public:
  /** The usualy used constructor.
  This constructor is the one that is usually used to convert default 
  C-Strings to i4_const_str. It can be called implicitly.
  @param ptr A string value.
  */
  i4_const_str(const char *ptr) : ptr(const_cast<char*>(ptr)) 
  { 
    if (ptr) 
      len=(w16)strlen(ptr); 
    else 
      len=0;
  }
  /** The default constructor, generates an empty string */
  i4_const_str():ptr(0),len(0) {};
  /** A special-purpose constructor, use only if you know what you're doing! */
  i4_const_str(char *ptr, w16 len) : ptr(ptr), len(len) {}
  /** The copy constructor */
  i4_const_str(const i4_const_str &str) : ptr(str.ptr), len(str.len) {}
  
  /** The iterator class for strings.
  This class is used like a stl-iterator type class.
  It points to one element of the string. 
  */
  class iterator
  {
    friend class i4_const_str;
    friend class i4_str;
  protected:
    char_type *node; ///< The "node" (character) this iterator points to. 
  public:
	  /** default constructor, don't use directly */
    iterator() : node(0) {}
	/** Copy constructor.
    Important for iterators.
	*/
    iterator(const iterator &p) : node(p.node) {}
	/** Creates an iterator that points to the given location of a string */
    iterator(char_type *p) : node(p) {}
	/** A special purpose constructor */
    explicit iterator(const iterator p, int i): node(p.node+i){}
	/** The equality operator for iterators */
    int operator==(const iterator p) const { return (node == p.node); }
	/** The inequality operator for iterators */
    int operator!=(const iterator p) const { return (node != p.node); }
    
    iterator& operator++() {     node++;  return *this; }
    //postfix operators must not return a reference type
    iterator operator++(int) { iterator it=*this; node++; return it; }
    iterator& operator--()  {     node--;  return *this; }
    iterator operator--(int) { iterator it=*this; node--; return it;}
    iterator operator+(const sw32 other)    {  return iterator(node+other); }
    iterator operator+=(const sw32 len) {   node+=len;   return *this;}
    iterator operator-(const sw32 other)   { return iterator(node-other);}
    iterator operator-=(const sw32 len) { node-=len; return *this;}

	/** Gets the character this iterator points to */
    i4_char get() const { return i4_char(*node); }

	/** Also gets the contents of the iterator */
    i4_char operator* () const { return get(); }

	/** Converts everything from here to the end of the string to an i4_str.
	The return value is an i4_str, so it becomes modifiable.
	*/
    i4_str *read_string();
	/** Returns a C-string */ 
    w32     read_ascii(char *buffer, w32 buffer_size);  // returns bytes read
	/** Attempts to convert the string to a number.
	@param throwexception Set to true if you want an error to be reported when the conversion fails.
	@return a number.
	*/
    sw32    read_number(i4_bool throwexception=i4_F);
	/** Returns a double value. 
	@see read_number()
	*/
    double  read_float(i4_bool throwexception=i4_F);
  };

  /** The destructor (only virtual function of the class) */
  virtual ~i4_const_str(){};

  /** Returns an iterator pointing to the one-beyond-the-last element of the string */
  const iterator end()   const { return ptr+len; }
  iterator end() { return ptr+len;}
  /** Returns the beginning of the string */
  const iterator begin() const { return ptr; }
  iterator begin() {return ptr;}

  //! Number of characters in the String.
  w32 length() const { return len; }
  //! For compatibiliy reasons, same as length() 
  w32 size() const {return len;}
  /** Generic length function.
   ascii length includes null-terminator and may eventually be longer than length()+1
   when kanji support is added and escape characters are used 
  */
  w32 ascii_length() const { return len+1; } 

  /** Difference between two iterators */
  swptr ptr_diff(const iterator first, const iterator last) const
  { return last.node-first.node; }
  /** Compares two strings */
  i4_bool operator== (const i4_const_str &other) const 
  { 
    if (len!=other.len)
      return 0;
    else return ::strncmp(other.ptr,ptr,len)==0; 
  }

  i4_bool operator!= (const i4_const_str &other) const 
  { return !(other==*this); }

  i4_bool operator<(const i4_const_str &other) const
      {
      return ::strncmp(other.ptr,ptr,len)>0?i4_T:i4_F;
      }

  i4_bool operator>(const i4_const_str &other) const
      {
      return ::strncmp(other.ptr,ptr,len)<0?i4_T:i4_F;
      }
  
  int strncmp(const i4_const_str &other, w32 max_cmp) const
  {
    return ::strncmp(ptr,other.ptr,max_cmp); 
  }
  /** Returns true if this string is null */
  i4_bool null() const { return (i4_bool)(ptr==0); }

  /// Pattern expansion.
  /// max_length is the maximum expanded length sprintf will create.
  /// sprintf uses it's own internal string as the format, and returns the
  /// result of the sprintf operation as a new i4_str
  /// example : i4_str *new_str=i4gets("format_str").sprintf(100,some_number);
  i4_str *sprintf(w32 max_length, ...) const;
  i4_str *vsprintf(w32 max_length, va_list &ap) const;

  /** Find some substring */
  iterator strstr(const i4_const_str &needle_to_find) const;
  
  ///Return the C representation of this string. 
  ///This function is implemented for compatibility with stl. 
  ///Do NOT delete the return value!
  ///The method newer returns 0.
  char_type *c_str() const //c_str() must not return 0
  {
  if (ptr)
  	return ptr;
  else return "";
  }
    
  char_type& operator[](int i) const {return *(ptr+i);}
};

/** The class for modifiable strings.
  This class is almost identical to the stl::string class.
  It supports all kinds of modifications, searches, concatenations and
  weird stuff. 
*/

class i4_str : public i4_const_str
{
  friend i4_str *i4_from_ascii(const char *buf);
protected:
  w16 buf_len;

  i4_str(int _buf_len) { alloc(_buf_len); }
  i4_str(w16 _buf_len) { alloc(_buf_len); }

  void alloc(w16 _buf_len);  
  void init_from_string(const i4_const_str &str, w16 _buf_len);

public:
  class iterator : public i4_const_str::iterator
  {
  protected:
  public:
    iterator(const iterator &p) : i4_const_str::iterator(p) {}
    explicit iterator(const iterator p, int i):i4_const_str::iterator(p,i) {}
    iterator(char_type *p) : i4_const_str::iterator((char_type *)p) {}
    void set(i4_char ch) { *((char_type *)node)=(char_type)ch.value(); }
	char_type& operator*() {return *((char_type*)node);}
  };
  i4_str():i4_const_str(),buf_len(0){}
  i4_str(const i4_str &str) : i4_const_str(0) { init_from_string(str, (w16)str.length()); }
  i4_str(const i4_const_str &str) : i4_const_str(0) { init_from_string(str, (w16)str.length()); }
  i4_str(const i4_const_str &str, w16 _len) : i4_const_str(0) { init_from_string(str, _len); }
  i4_str(const char *s);
  i4_str(const char c);
  // copies from start to end-1 characters (does not include end)
  i4_str(const i4_const_str::iterator start, const i4_const_str::iterator end, w16 buf_len);

  /** Insertion and deletion operators.
  These methods might change this.ptr! 
  when the functions return, p still points to same character, but node 
  might have changed anyway, meaning the iterator is no more valid.
  First kind: Insert other before p.
  \par Warning
  Insert invalidates all iterators pointing to the string. 
  Do not use code like the following.
  \code
  i4_str str("my tiny little string");
  i4_str::iterator it=str.begin();
  str.insert(it,"is ");
  str.insert(it,"This "); //bang! it is no more valid.
  \endcode
  */
  void insert(i4_str::iterator p, const i4_const_str &other);   // insert other before p
  /** More insertion.
  Second kind: insert ch before p
  */
  void insert(i4_str::iterator p, const i4_char ch);            // insert ch before p
  /** Deletion.
  Delete all from start to (but not including) last.
  */
  void remove(i4_str::iterator start, i4_str::iterator last);
  i4_str operator+(i4_str& a)
  	{
	i4_str tmp(*this);
	tmp.insert(end(),a);
	return tmp;
	}
  /** Concatenates two strings.
  The first parameter must not be passed by reference!
  */
  friend i4_str operator+(i4_str a, i4_str b)
	  {
	  a.insert(a.end(),b);
	  return a;
	  }
  /** The assignment operator */
  i4_str& operator=(const i4_str &other)
  	{
	if (ptr)
		i4_free((char*)ptr);
	ptr=0;len=0;
	init_from_string(other,(w16)other.length());
	return *this;
	} 
  i4_str& operator+=(const i4_str &a)
  	{
	(*this).insert(end(),a);
	return *this;
	}
  int find(const i4_str &str) const;
  //! Finds the first occurence of any of the elements of str.
  int find_first_of(const i4_str &str) const;
  //! Finds the last occurence of any of the elements of str. 
  int find_last_of(const i4_str &str) const;
  i4_str substr(int start, int &len) const;
  int find_first_not_of(const i4_str &str) const;
  int find_last_not_of(const i4_str &str) const;
  //!erases everything from start (included) to end (exclusive)
  void erase(i4_str::iterator start, i4_str::iterator last)
  	{remove (start, last);}
  //!erases bytes from start to end. 
  void erase(int start, int bytes);
  	
  void to_upper();   ///< converts all the chars in this string to upper case
  void to_lower();   ///< converts all the chars in this string to lower case
  const iterator end()   const { return ptr+len; }
  iterator end() {return ptr+len;}
  const iterator begin() const { return ptr; }
  iterator begin() {return ptr;}
  ///this function must only be used if you know that the string-memory
  ///is large enough, i.e it has been created with the (str,len) constructor.
  void set_length(int l) { len=l; }
  public:
	  /// The virtual destructor 
  virtual ~i4_str();

};


class i4_linear_allocator;
class i4_grow_heap_class;
class i4_string_manager_class : public i4_init_class
{  
private:
  i4_grow_heap_class *string_heap;
  char  *alloc_str(char *string);
  void add_node(char *token, char *string);
  void add_array_node(char *token, char **array, w32 total);
  friend class i4_string_manager_saver_class;
  class node
  {
  public:
    typedef i4_linear_allocator node_allocator;
    static node_allocator *nodes;
    static w32 nodes_ref;           // number of string managers using 'nodes'
    char *str_token;
    i4_const_str value;
    node *left,*right;

//#ifndef i4_NEW_CHECK
    void *operator new(size_t size);
    void operator delete(void *ptr);
//#endif
    node(char *token, const i4_const_str &value) : str_token(token), value(value)
    {
      left=0;
      right=0;
    }
    ~node();
  };
  node *root;
#ifdef __MAC__
  node *new_node(char *token, const i4_const_str &value) 
  { 
    node *p = (node *)node::nodes->alloc();
    p->str_token = token;
    p->value = value;
    p->left = 0;
    p->right = 0;
    return p;
  }
  void delete_node(node *n)
  {
    if (n->left)
      delete_node(n->left);
    if (n->right)
      delete_node(n->right);
    node::nodes->free(n);
  }
#else
  node *new_node(char *token, const i4_const_str &value) { return new node(token,value); }
  void delete_node(node *n) { delete n; }
#endif
  class array_node
  {
  public:
    typedef i4_linear_allocator node_allocator;
    static node_allocator *nodes;
    
    char *str_token;
    char **value;
    array_node *left,*right;
//#ifndef i4_NEW_CHECK
    void *operator new(size_t size);
    void operator delete(void *ptr);
//#endif
    array_node(char *token, char **value) : str_token(token),value(value)
    {
      left=0;
      right=0;
    }
    ~array_node();
  } *array_root;

#ifdef __MAC__
  array_node *new_array_node(char *token, char **value) 
  { 
    array_node *p = (array_node *)array_node::nodes->alloc();
    p->str_token = token;
    p->value = value;
    p->left = 0;
    p->right = 0;
    return p;
  }

  void delete_array_node(array_node *n)
  {
    if (n->left)
      delete_array_node(n->left);
    if (n->right)
      delete_array_node(n->right);
    array_node::nodes->free(n);
  }
#else
  array_node *new_array_node(char *token, char **value) 
  { 
    return new array_node(token,value); 
  }
  void delete_array_node(array_node *n) { delete n; }
#endif
  void get_token(char *&s, char *&buf, w32 &line_on, char *error_prefix);
  void expand_macro(char *&s, char *&buf, w32 &line_on, char *error_prefix);
  void read_array(char *&s, 
                  char **array, 
                  w32 &total,
                  w32 &line_on, 
                  char *error_prefix, 
                  char *token_buf);
                  
  void get_char(char *&s, char *&buf, w32 &line_on, char *error_prefix);

  void show_node(node *who);
  void show_nodes();
public:
  i4_string_manager_class();
  ~i4_string_manager_class();
  int init_type() { return I4_INIT_TYPE_STRING_MANAGER; }
  void init();
  void uninit();

  i4_bool       load(char *filename);
  i4_bool       load(const i4_const_str &filename);
  i4_bool       load_buffer(void *internal_buffer, char *error_prefix);

  const i4_const_str &get(const char *internal_name);
  const i4_const_str &get(const i4_const_str &internal_name);

  i4_const_str *get_array(const char *internal_name);
  i4_const_str *get_array(const i4_const_str &internal_name);
};


// "the" main string manager for a program
extern i4_string_manager_class i4_string_man;

const i4_const_str &i4gets(char *str, i4_bool barf_on_error=i4_T);
int i4getn(char *str, i4_bool barf_on_error=i4_T);

// converts an i4_const_str to an 8 bit ascii string (where possible)
extern char *i4_os_string(const i4_const_str &name, char *buffer, int buflen);

// converts from an ascii string to an i4_str

extern i4_str *i4_from_ascii(const char *buf);

#include "memory/new.h"
#endif



