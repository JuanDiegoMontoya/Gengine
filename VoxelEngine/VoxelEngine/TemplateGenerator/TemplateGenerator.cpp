// TemplateGenerator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <fstream>
#include <sstream>
#include <string>

std::string& replaceAll(std::string& context, std::string const& from, std::string const& to)
{
  std::size_t lookHere = 0;
  std::size_t foundHere;
  while ((foundHere = context.find(from, lookHere)) != std::string::npos)
  {
    context.replace(foundHere, from.size(), to);
    lookHere = foundHere + to.size();
  }
  return context;
}

int main(int argc, char* argv[])
{
  if (argc == 3)
  {
    std::string type = std::string(argv[1]);
    std::string name = std::string(argv[2]);
    if (type.length() != 1 && name.length() == 1)
    {
      std::string temp = name;
      name = type;
      type = temp;
    }
    if (type.length() == 1)
    {
      bool valid = true;

      char stubToGenerate = toupper(type[0]);
      std::string stubFileType;
      if      (stubToGenerate == 'E') stubFileType = "Event";
      else if (stubToGenerate == 'S') stubFileType = "System";
      else if (stubToGenerate == 'M') stubFileType = "Manager";
      else valid = false;

      if (valid)
      {
        std::ifstream ifs;
        std::ofstream ofs;

        std::string nameHeaderPath = std::string("Headers/") + stubFileType + "s/" + name + stubFileType + ".h";
        std::string stubHeaderPath = std::string("Headers/") + stubFileType + "s/Stub"    + stubFileType + ".h";
        std::string nameSourcePath = std::string("Source/")  + stubFileType + "s/" + name + stubFileType + ".cpp";
        std::string stubSourcePath = std::string("Source/")  + stubFileType + "s/Stub"    + stubFileType + ".cpp";

        ifs.open(stubHeaderPath);
        std::string fileContents;
        if (ifs) 
        {
          std::ostringstream oss;
          oss << ifs.rdbuf();
          fileContents = oss.str();

          replaceAll(fileContents, std::string("Stub") + stubFileType, name);

          ofs.open(nameHeaderPath);
          ofs << fileContents;
          ofs.close();
          ifs.close();
        }
        if (stubToGenerate != 'E')
        {

          ifs.open(stubSourcePath);
          if (ifs)
          {
            std::ostringstream oss;
            oss << ifs.rdbuf();
            fileContents = oss.str();

            replaceAll(fileContents, std::string("Stub") + stubFileType, name);

            ofs.open(nameSourcePath);
            ofs << fileContents;
            ofs.close();
            ifs.close();
          }
        }
        std::cout << "GG you generated your new " << name << " class, and have proven you can follow the simplest of instructions\nJust be sure to add it to the Visual Studio Solution under the proper filter and you'll be good to go.\n";
        system("..\\x64\\Release\\PrePP.exe > nul 2>&1");
        return 0;
      }
    }
  }

  std::cout << "Error.\nJust put:\n\nTemplateGenerator [] []\n\nwhere each one of the box brackets is replaced with:\n"
    << "E for a new Event\nM for a new Manager\nS for a new System\n\nThe other box bracket is replaced with whatever name you want.\nIt just has to be more than 1 character plz.\n"
    << "\nHave a good day and don't screw it up next time.\n";
  return 1;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
