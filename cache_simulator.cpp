/*
*COMPUTER ORGANIZATION AND ARCHITECTURE LAB
*CACHE SIMULATOR
*WRITTEN BY:	CS19B022	KATTA MANASA
*		CS19B057	ENNA SAI TEJA
*		CS19B070	RAHUL BIJARNIYA
*/

#include <iostream>
#include <list>
#include <cmath>
#include <unordered_map>
#include <string.h>
#include <fstream>
#include <vector> 
using namespace std;

struct misses{
								// initialise all stats of cache		
  int cache_misses = 0;
  int compulsory_misses = 0;
  int capacity_misses = 0;
  int conflict_misses = 0;
  int read_misses = 0;
  int write_misses = 0;
  int dirty_bevicted = 0;  
};

struct access{
    int cache_access = 0;
    int read_access = 0;
    int write_access = 0;
};

struct cacheBlock{
    int tag;
    int valid_bit;
    int dirty_bit; 
};

int getnumbits(int size)
{
    return (int)log2(size);
}

void update(int arr[],int x,int k)
{
   while(x!=0)
   {
      if(x%2 == 1)
      {
         arr[k] = 1;
      }
      x = x/2;
      k--;
   }
}

int* convert(string hexa){						
   long int i = 2,k=3;
   int* arr;
   
   arr = new int[4*(hexa.length())-8];
   for(int i=0;i<4*(hexa.length())-8;i++)
   {
   arr[i] = 0;
   }
   while (hexa[i]){
      int x = hexa[i]-'0';
      if(x>9||x<0)
      {
           int k = (hexa[i]>=97) ? 97:65;
         x = hexa[i]-k+10;
      }
      update(arr,x,k);
      k= k+4;
   i++;
   }
   return arr;
}

int convertbinary(int arr[],int l,int h)
{
  int res = 0;
 for(int i=h;i>=l;i--)
 {
     res = res + arr[i]*pow(2,h-i);
 }
  return res;
}

class Cache{
 public:
    misses m;
    access a;
    int cache_size;
    int block_size;
    int associtivity;
    int set_num;
    int block_num;
    int tag_len;
    int capacity;
    list<cacheBlock>* set;
    vector<vector<int>> tree;
    vector<vector<cacheBlock>> node;
    unordered_map<int,int> h;
    Cache(int cache_size,int block_size,int associtivity,int p);
    void print();
    void input(string s,int r_w,int p);
    void LRU(int set_ind,int tagn,int set_capacity,int r_w,bool access);
    void pseudoLRU(int set_ind,int tagn,int set_capacity,int r_w,bool access);
    void Random(int set_ind,int tagn,int set_capacity,int r_w,bool access);

};


void Cache::input(string s,int r_w,int p)
{
    int *address;
    address = convert(s);
    int set_ind;
    int tagn;
    bool access;
    int set_capacity;
    a.cache_access++;
    
   tagn    = convertbinary(address,0,tag_len-1);
   int k   = convertbinary(address,0,tag_len+getnumbits(set_num)-1);
    if(h[k] == 0)
       {
           h[k]++;
           access = false;
       }
       else
       {
           access = true;
       }
   if(associtivity != 0)
    {
      set_ind = convertbinary(address,tag_len, tag_len + getnumbits(set_num)-1);
      set_capacity = associtivity;
    }
    else
    {
        set_ind      = 0;
        set_capacity = block_num;
    }
    if(p == 0)
    {
        Random(set_ind,tagn,set_capacity,r_w,access);
    }
    else if(p == 1)
    {
         LRU(set_ind,tagn,set_capacity,r_w,access);
    }
    else
    {
        pseudoLRU(set_ind,tagn,set_capacity,r_w,access);
    }
   
   
}
Cache::Cache(int cache_size,int block_size,int associtivity,int p)
{											// initialise all information of cache 
    this->cache_size   = cache_size;
    this->block_size   = block_size;
    this->associtivity = associtivity;
    block_num = cache_size/block_size;
    
    set_num = (associtivity==0) ? 1 : (block_num/associtivity);
    tag_len  = 32 - (getnumbits(block_size)+getnumbits(set_num)); 
    											// calculate number of sets for given cache
    if(p == 1)
    {
       set = new list<cacheBlock>[set_num]; 
    }
    else
    {
        int p1,p2;
        cacheBlock temp;
        temp.valid_bit = 0;
        temp.dirty_bit = 0;
        temp.tag       = -1;
        p2 = (associtivity == 0) ? block_num : associtivity;
        p1 = (p2 == 1) ? 1 : p2-1;
       vector<cacheBlock> t;
       int k=0;
        vector<int> tem;
         for(int j=0;j<p2;j++)
          {
              t.push_back(temp);
          }
          for(int j=0;j<p1;j++)
          {
              tem.push_back(k);
          }
      for(int i=0;i<set_num;i++)
      {
          node.push_back(t);
          if(p == 2)
          {
              tree.push_back(tem);
          }
      }
    }
}

void Cache::print()						// display statistics as described	
{
    cout<<"cache access : "<<a.cache_access<<endl;
    cout<<"read access : "<<a.read_access<<endl;
    cout<<"write access : "<<a.write_access<<endl;
    cout<<"cache misses : "<<m.cache_misses<<endl;
    cout<<"compulsory misses : "<<m.compulsory_misses<<endl;
    cout<<"capacity misses : "<<m.capacity_misses<<endl;
    cout<<"conflict misses : "<<m.conflict_misses<<endl;
    cout<<"read misses : "<<m.read_misses<<endl;
    cout<<"write misses : "<<m.write_misses<<endl;
    cout<<"dirty blocks evicted : "<<m.dirty_bevicted<<endl;
}

void Cache::LRU(int set_ind,int tagn,int set_capacity,int r_w,bool access)
{
    
    cacheBlock temp;
   
    for(auto it = set[set_ind].begin();it!= set[set_ind].end();it++)		 // loop to check if request is present or not 
        {
            temp = *it;
            if(temp.valid_bit == 1 && temp.tag == tagn)
            {
               
                set[set_ind].erase(it);
                if(r_w == 0)
                {
                    a.read_access ++;
                     
                }
                else
                {
                    a.write_access++;
                   temp.dirty_bit = 1;
                }
                
                set[set_ind].insert(set[set_ind].begin(),temp);
                return;
            }
        }
        
    temp.valid_bit = 1;
    temp.tag       = tagn;
    m.cache_misses++;
    if(r_w == 0)
            {
                m.read_misses++;
                a.read_access++;
                temp.dirty_bit = 0;
            }
            else
            {
                m.write_misses++;
                a.write_access++;
               temp.dirty_bit = 1;
            }
    long int t = set[set_ind].size(); 
    if(t< set_capacity)
    {
            m.compulsory_misses++;
            set[set_ind].insert(set[set_ind].begin(),temp);
            capacity++;
             return;
    }
    
    else
    {
        if(access)
        {
           m.conflict_misses++;
        }
       else
       {
           m.compulsory_misses++;
       }
        
        if(capacity == block_num)
        {
            m.capacity_misses++;
        }
        
        auto it1 = set[set_ind].end();
        it1--;
        cacheBlock evict = *it1;
        if(evict.dirty_bit == 1)
        {
            m.dirty_bevicted++;
        }
        
        set[set_ind].erase(it1);
        set[set_ind].insert(set[set_ind].begin(),temp);
    }
}

void Cache::pseudoLRU(int set_ind,int tagn,int set_capacity,int r_w,bool access)
{
  bool left;
    int parent;
    int temp;
   							
    for(int i=0;i<set_capacity;i++)						// loop to check if request is present or not 
    {
        
        if(node[set_ind][i].valid_bit == 1 && node[set_ind][i].tag == tagn)
        {
            temp = (set_capacity-1)+i;
            
           while(true)
            {
              
                if(temp%2 == 0)
              {
                left = false;
              }
                else
               {
                left = true;
               }
                parent = (temp-1)/2;
                if(tree[set_ind][parent] == 0 && left)
                {
                    tree[set_ind][parent] = 1;
                }
                else if(tree[set_ind][parent] == 1 && !left)
                {
                    tree[set_ind][parent] = 0;
                }
               if(parent == 0)
               {
                   break;
               }
                temp = (temp-1)/2;
               
            }
            
            if(r_w == 0)
                {
                    a.read_access++;
                     
                }
                else
                {
                    a.write_access++;
                   node[set_ind][i].dirty_bit = 1;
                }
            return;
        }
    }
    temp = 0;
    
  while(2*temp+1 < (2*(set_capacity)-1))
    {
     
        parent = tree[set_ind][temp];
        if(parent == 0)
        {
            
            tree[set_ind][temp]  = 1;
           
            temp = 2*temp +1;
           
        }
        else 
        {
          
          tree[set_ind][temp]  = 0;
           temp = 2*temp +2;
          
        }
    }
   
    int j =temp+ 1-set_capacity;
    if(capacity == block_num)
        {
            m.capacity_misses++;
        }
    capacity++;
     if (node[set_ind][j].valid_bit ==  1)
           {
                 capacity--;
                 if(node[set_ind][j].dirty_bit == 1)
                 {
                    m.dirty_bevicted++;
                 }
           }
    
           if(access)
           {
               m.conflict_misses++;
           }
    
          else
          {
            m.compulsory_misses++;
          }
           m.cache_misses++;
       node[set_ind][j].tag = tagn;
       node[set_ind][j].valid_bit = 1;
       
        
        if(r_w == 0)
        {
            m.read_misses++;
            a.read_access++;
            node[set_ind][j].dirty_bit = 0;
        }
        else
        {
            a.write_access++;
            m.write_misses++;
            node[set_ind][j].dirty_bit = 1;
        }
        
}

void Cache::Random(int set_ind,int tagn,int set_capacity,int r_w,bool access)
{
     
    for(int i=0;i<set_capacity;i++)
    {
        if(node[set_ind][i].valid_bit == 1 && node[set_ind][i].tag == tagn)
        {
              if(r_w == 0)
                {
                    a.read_access++;
                     
                }
                else
                {
                    a.write_access++;
                   node[set_ind][i].dirty_bit = 1;
                }
            return;
        }
    }
    int temp = 0;
     m.cache_misses++;
    while(temp <set_capacity && node[set_ind][temp].valid_bit == 1)
    {
        temp++;
    }
    
    if(temp != set_capacity)
    {
       
        m.compulsory_misses++;
        capacity++;
    }
    else
    {
        temp = rand()%set_capacity;
        if(node[set_ind][temp].dirty_bit == 1)
        {
            m.dirty_bevicted++;
        }
        if(access)
        {
            m.conflict_misses++;
        }
        else
        {
            m.compulsory_misses++;
        }
        if(capacity == block_num)
        {
            m.capacity_misses++;
        }
    }
    
     node[set_ind][temp].valid_bit = 1;
        node[set_ind][temp].tag = tagn;
        if(r_w == 0)
        {
            m.read_misses++;
            a.read_access++;
            node[set_ind][temp].dirty_bit = 0;
        }
        else
        {
            a.write_access++;
            m.write_misses++;
            node[set_ind][temp].dirty_bit = 1;
        }
}

int main()
{
    int a,b,c,p;
    fstream file;
    file.open("input.txt",ios::in);
    file>>a;
    file>>b;
    file>>c;
    file>>p;
    
     Cache l(a,b,c,p);
      while(true)
        {
          string s1,s2;
          file>>s1>>s2;
         
          if(file.eof())
           {
             break;
           }
     
          if(s2 == "r")
            {
              l.input(s1,0,p);
            }
          else
            {
              l.input(s1,1,p);
            }
        }

    l.print();
  file.close();
   return 0;
}

