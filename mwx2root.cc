/** Program to convert MWX format from radiosondes to ROOT trees 
 *
 *  Cosmin Deaconu
 *  <cozzyd@kicp.uchicago.edu> 
 *
 **/ 


#include "TTree.h" 
#include "TSystem.h" 
#include "TFile.h" 
#include "TTimeStamp.h" 
#include <stdlib.h> 
#include "TString.h" 
#include "TDOMParser.h" 
#include <vector> 
#include <string> 
#include <iostream> 
#include <unordered_map> 
#include "TXMLNode.h"
#include "TXMLAttr.h"





std::vector<std::string> xml_files; 
std::string ofile = ""; 
std::string ifile = ""; 


void usage()
{
  std::cout << " mwx2root file.mwx [-o output=file.root] [XMLFilePrefix = GpsResults] [AnotherXMLFilePrefix] [...] " << std::endl; 
}


double parseTime(const char * str) 
{
  
  double secs; 
  int yr, mon,day,hr,min; 
  int matched = sscanf(str,"%04d-%02d-%02dT%02d:%02d:%lf", &yr, &mon,&day, &hr,&min,&secs); 
  if (matched != 6) return -1; 

  TTimeStamp t(yr,mon,day,hr,min,int(secs), (secs-int(secs))*1e9); 
  return t.AsDouble(); 
}


void makeTree(TFile & of, std::string & xmlfile) 
{
  std::string cmd = "unzip -cq " + ifile + " " + xmlfile + ".xml" ; 

  TString output = gSystem->GetFromPipe(cmd.c_str()); 

  if (!output.Length())
  {
    std::cerr << xmlfile << ".xml could not be read from " << ifile << std::endl; 
    return; 
  }

  TDOMParser dom;  
  dom.SetValidate(false); 
  dom.ParseBuffer(output.Data(), output.Length()); 

  auto xml = dom.GetXMLDocument(); 

  auto rnode = xml->GetRootNode(); 

  of.cd(); 

  auto cnode = rnode->GetChildren(); 

  if (!cnode) 
  {
    std::cerr << xmlfile << ".xml from " << ifile << " has no child nodes." << std::endl; 
    return; 
  }
  else
  {
    while (cnode->GetNextNode()->HasNextNode()) cnode = cnode->GetNextNode(); 
  }

  TTree * t = new TTree(rnode->GetNodeName(), rnode->GetNodeName()); 

  //get attributes 

  std::unordered_map<std::string, int> attrs; 
  std::vector<int> is_time; 
  std::vector<int> is_str; 
  std::vector<double> dbl_vals; 
  std::vector<std::string> strvals; 
  //should be plenty... 
  dbl_vals.reserve(1000);
  strvals.reserve(1000);


  TIter next(cnode->GetAttributes()); 

  int attr_counter = 0; 
  TXMLAttr * attr = 0; 
  while ( (attr = (TXMLAttr*) next() )  )
  {
    //let's figure out what this is... 

    
    // is it a number? 

    char * endptr = 0; 
    double val = strtod(attr->GetValue(), &endptr); 
    dbl_vals.push_back(0); 
    strvals.push_back(""); 

    attrs[attr->GetName()] = attr_counter; 
    //not a double ... 
    if (*endptr) 
    {
      // is it a time? 

      double astime = parseTime(attr->GetValue());

      if (astime == -1)  // a string
      {
        is_str.push_back(true); 
        is_time.push_back(false); 
        t->Branch(attr->GetName(),&strvals[attr_counter]); 
        printf("%s is a string (%s)\n", attr->GetName(), attr->GetValue()); 
      }
      else   // a time 
      { 

        printf("%s is a time (%g)\n", attr->GetName(), astime); 
        is_time.push_back(true); 
        is_str.push_back(false); 
        t->Branch(attr->GetName(),&dbl_vals[attr_counter]); 
      }

    }
    else // a double
    {
      printf("%s is a double (%g)\n", attr->GetName(), val); 
      t->Branch(attr->GetName(), &dbl_vals[attr_counter]); 
      is_time.push_back(false); 
      is_str.push_back(false); 
    }


    attr_counter++;; 
  }

//  printf("%d %u %u %u %u\n", attr_counter, is_time.size(), is_str.size(), dbl_vals.size(), strvals.size()); 

  cnode = rnode->GetChildren()->GetNextNode(); 

  while (cnode->HasNextNode()) 
  {
    TIter it(cnode->GetAttributes()); 

    while ( (attr = (TXMLAttr*) it() )  )
    {
      int i = attrs[attr->GetName()]; 
//      printf("%s %d %d %d\n", attr->GetName(), i, is_str[i], is_time[i]); 

      if (is_time[i])
      {
        dbl_vals[i] = parseTime(attr->GetValue()); 
      }
      else if (is_str[i])
      {
        strvals[i] = attr->GetValue(); 
      }
      else 
      {
        dbl_vals[i] = strtod(attr->GetValue(),0); 
      }
    }

    t->Fill(); 

    cnode = cnode->GetNextNode(); 
  }




  t->Write();
}





int main(int nargs, const char ** args) 
{
  if (nargs < 2) 
  {
    usage(); 
    return 1; 
  }

  for (int i= 1; i <nargs; i++) 
  {
    if (std::string(args[i]) ==  "-o")
    {
      ofile = std::string(args[++i]); 
    }

    else if (ifile == "") 
    {
      ifile = args[i]; 
    }
    else
    {
      xml_files.push_back(args[i]); 
    }
  }

  
  if (!xml_files.size()) xml_files.push_back("GpsResults"); 

  if (ofile == "") 
  {
    auto pos = ifile.rfind(".mwx"); 
    if (pos == std::string::npos) 
    {
     ofile = ifile+".root"; 
    }
    else
    {
      ofile = ifile; 
      ofile.replace(pos,4,".root"); 
    }
  }


  TFile of(ofile.c_str(), "RECREATE"); 

  for (auto xml : xml_files) 
  {
    makeTree(of, xml); 
  }

  return 0; 
}

