#include<iostream> 
 
using namespace std;



bool isTrival(string input)
{
 // iinput.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
    if(input.compare(string(""))==0||input.compare(string(" "))==0||input.empty()||input.length()==0)
     {
        return true;
     }
     return false;
}

bool acontainsb(string a, string b)
{

  if (a.find(b) != std::string::npos) {
    std::cout << "found!" << '\n';
   }

}
 
// main function -
// where the execution of program begins
int main()
{
    string test1="yes yan1";
    string test2="yan";
    if(acontainsb(test1,test2))
{
   cout<<"hey I am correct!!"<<endl;
   }
    return 0;
}

