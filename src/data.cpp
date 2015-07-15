/**********************************************************************
data.cpp - Global data and resource file parsers.

Copyright (C) 1998-2001 by OpenEye Scientific Software, Inc.
Some portions Copyright (C) 2001-2008 by Geoffrey R. Hutchison

This file is part of the Open Babel project.
For more information, see <http://openbabel.org/>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
***********************************************************************/

#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#include <openbabel/babelconfig.h>
#include <openbabel/data.h>
#include <openbabel/mol.h>
#include <openbabel/locale.h>

// data headers with default parameters
#include "element.h"
#include "types.h"
#include "isotope.h"
#include "resdata.h"


#if !HAVE_STRNCASECMP
extern "C" int strncasecmp(const char *s1, const char *s2, size_t n);
#endif

using namespace std;

namespace OpenBabel
{

  OBElementTable   etab;
  OBTypeTable      ttab;
  OBIsotopeTable   isotab;
  OBResidueData    resdat;

  /** \class OBElementTable data.h <openbabel/data.h>
      \brief Periodic Table of the Elements

      Translating element data is a common task given that many file
      formats give either element symbol or atomic number information, but
      not both. The OBElementTable class facilitates conversion between
      textual and numeric element information. An instance of the
      OBElementTable class (etab) is declared as external in data.cpp. Source
      files that include the header file mol.h automatically have an extern
      definition to etab. The following code sample demonstrates the use
      of the OBElementTable class:
      \code
      cout << "The symbol for element 6 is " << etab.GetSymbol(6) << endl;
      cout << "The atomic number for Sulfur is " << etab.GetAtomicNum(16) << endl;
      cout << "The van der Waal radius for Nitrogen is " << etab.GetVdwRad(7);
      \endcode

      Stored information in the OBElementTable includes elemental:
      - symbols
      - covalent radii
      - van der Waal radii
      - expected maximum bonding valence
      - molar mass (by IUPAC recommended atomic masses)
      - electronegativity (Pauling and Allred-Rochow)
      - ionization potential
      - electron affinity
      - RGB colors for visualization programs
      - names (by IUPAC recommendation)
  */

  OBElementTable::OBElementTable()
  {
    _init = false;
    _dir = BABEL_DATADIR;
    _envvar = "BABEL_DATADIR";
    _filename = "element.txt";
    _subdir = "data";
    _dataptr = ElementData;
  }

  OBElementTable::~OBElementTable()
  {
    vector<OBElement*>::iterator i;
    for (i = _element.begin();i != _element.end();++i)
      delete *i;
  }

  void OBElementTable::ParseLine(const char *buffer)
  {
    int num,maxbonds;
    char symbol[4];
    char name[256];
    double Rcov,Rvdw,mass, elNeg, ARENeg, ionize, elAffin;
    double red, green, blue;

    if (buffer[0] != '#') // skip comment line (at the top)
      {
        sscanf(buffer,"%d %4s %lf %lf %*f %lf %d %lf %lf %lf %lf %lf %lf %lf %255s",
               &num,
               symbol,
               &ARENeg,
               &Rcov,
               &Rvdw,
               &maxbonds,
               &mass,
               &elNeg,
               &ionize,
               &elAffin,
               &red,
               &green,
               &blue,
               name);

        OBElement *ele = new OBElement(num,symbol,ARENeg,Rcov,Rvdw,maxbonds,mass,elNeg,
                                       ionize, elAffin, red, green, blue, name);
        _element.push_back(ele);
      }
  }

  unsigned int OBElementTable::GetNumberOfElements()
  {
    if (!_init)
      Init();

    return _element.size();
  }

  const char *OBElementTable::GetSymbol(int atomicnum)
  {
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return("\0");

    return(_element[atomicnum]->GetSymbol());
  }

  int OBElementTable::GetMaxBonds(int atomicnum)
  {
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return(0);

    return(_element[atomicnum]->GetMaxBonds());
  }

  double OBElementTable::GetElectroNeg(int atomicnum)
  {
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return(0.0);

    return(_element[atomicnum]->GetElectroNeg());
  }

  double OBElementTable::GetAllredRochowElectroNeg(int atomicnum)
  {
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return(0.0);

    return(_element[atomicnum]->GetAllredRochowElectroNeg());
  }


  double OBElementTable::GetIonization(int atomicnum)
  {
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return(0.0);

    return(_element[atomicnum]->GetIonization());
  }


  double OBElementTable::GetElectronAffinity(int atomicnum)
  {
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return(0.0);

    return(_element[atomicnum]->GetElectronAffinity());
  }

  vector<double> OBElementTable::GetRGB(int atomicnum)
  {
    if (!_init)
      Init();

    vector <double> colors;
    colors.reserve(3);

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      {
        colors.push_back(0.0);
        colors.push_back(0.0);
        colors.push_back(0.0);
        return(colors);
      }

    colors.push_back(_element[atomicnum]->GetRed());
    colors.push_back(_element[atomicnum]->GetGreen());
    colors.push_back(_element[atomicnum]->GetBlue());

    return (colors);
  }

  string OBElementTable::GetName(int atomicnum)
  {
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return("Unknown");

    return(_element[atomicnum]->GetName());
  }

  double OBElementTable::GetVdwRad(int atomicnum)
  {
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return(0.0);

    return(_element[atomicnum]->GetVdwRad());
  }

  double OBElementTable::CorrectedBondRad(int atomicnum, int hyb)
  {
    double rad;
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return(1.0);

    rad = _element[atomicnum]->GetCovalentRad();

    if (hyb == 2)
      rad *= 0.95;
    else if (hyb == 1)
      rad *= 0.90;

    return(rad);
  }

  double OBElementTable::CorrectedVdwRad(int atomicnum, int hyb)
  {
    double rad;
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return(1.95);

    rad = _element[atomicnum]->GetVdwRad();

    if (hyb == 2)
      rad *= 0.95;
    else if (hyb == 1)
      rad *= 0.90;

    return(rad);
  }

  double OBElementTable::GetCovalentRad(int atomicnum)
  {
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return(0.0);

    return(_element[atomicnum]->GetCovalentRad());
  }

  double OBElementTable::GetMass(int atomicnum)
  {
    if (!_init)
      Init();

    if (atomicnum < 0 || atomicnum >= static_cast<int>(_element.size()))
      return(0.0);

    return(_element[atomicnum]->GetMass());
  }

  int OBElementTable::GetAtomicNum(const char *sym)
  {
    int temp;
    return GetAtomicNum(sym, temp);
  }

  int OBElementTable::GetAtomicNum(const char *identifier, int &iso)
  {
    if (!_init)
      Init();

    char buffer[BUFF_SIZE]; // error buffer

    // Compare to symbol
    vector<OBElement*>::iterator i;
    for (i = _element.begin();i != _element.end();++i)
      if (!strncasecmp(identifier,(*i)->GetSymbol(),3))
        return((*i)->GetAtomicNum());

    // Compare to IUPAC name (an abbreviated name will also work if 5 letters or more)
    int numCharsToTest = std::max<int>(strlen(identifier), 5);
    for (i = _element.begin();i != _element.end();++i)
      if (strncasecmp(identifier,(*i)->GetName().c_str(),numCharsToTest) == 0)
        return((*i)->GetAtomicNum());

    if (strcasecmp(identifier, "D") == 0 ||
        (strcasecmp(identifier, "Deuterium") == 0) )
      {
        iso = 2;
        return(1);
      }
    else if (strcasecmp(identifier, "T") == 0 ||
             (strcasecmp(identifier, "Tritium") == 0) )
      {
        iso = 3;
        return(1);
      }
    else if (strcasecmp(identifier, "Hl") == 0) // ligand hydrogen -- found in some CIF PR#3048959.
      {
        snprintf(buffer, BUFF_SIZE, "Cannot understand the element label %s. Guessing it's hydrogen.", identifier);
        obErrorLog.ThrowError(__FUNCTION__, buffer, obWarning);
        return(1);
      }
    else
      iso = 0;

    if(identifier[0]!='*')
      {
        snprintf(buffer, BUFF_SIZE, "Cannot understand the element label %s.", identifier);
        obErrorLog.ThrowError(__FUNCTION__, buffer, obWarning);
      }
    return(0);
  }

  int OBElementTable::GetAtomicNum(string name, int &iso)
  {
    return GetAtomicNum(name.c_str(), iso);
  }

  /** \class OBIsotopeTable data.h <openbabel/data.h>
      \brief Table of atomic isotope masses
  */

  OBIsotopeTable::OBIsotopeTable()
  {
    _init = false;
    _dir = BABEL_DATADIR;
    _envvar = "BABEL_DATADIR";
    _filename = "isotope.txt";
    _subdir = "data";
    _dataptr = IsotopeData;
  }

  void OBIsotopeTable::ParseLine(const char *buffer)
  {
    unsigned int i;
    vector<string> vs;

    pair <unsigned int, double> entry;
    vector <pair <unsigned int, double> > row;

    if (buffer[0] != '#') // skip comment line (at the top)
      {
        tokenize(vs,buffer);
        if (vs.size() > 3) // atomic number, 0, most abundant mass (...)
          {
            for (i = 1; i < vs.size() - 1; i += 2) // make sure i+1 still exists
              {
                entry.first = atoi(vs[i].c_str()); // isotope
                entry.second = atof(vs[i + 1].c_str()); // exact mass
                row.push_back(entry);
              }
            _isotopes.push_back(row);
          }
        else
          obErrorLog.ThrowError(__FUNCTION__, " Could not parse line in isotope table isotope.txt", obInfo);
      }
  }

  double        OBIsotopeTable::GetExactMass(const unsigned int ele,
                                             const unsigned int isotope)
  {
    if (!_init)
      Init();

    if (ele > _isotopes.size())
      return 0.0;
    if (_isotopes[ele].size() == 0)
      return 0.0; // PR#2996661

    unsigned int iso;
    for (iso = 0; iso < _isotopes[ele].size(); ++iso)
      if (isotope == _isotopes[ele][iso].first)
        return _isotopes[ele][iso].second;

    return 0.0;
  }

  OBAtomicHeatOfFormationTable::OBAtomicHeatOfFormationTable(void)
  {
    _init = false;
    _dir = BABEL_DATADIR;
    _envvar = "BABEL_DATADIR";
    _filename = "atomization-energies.txt";
    _subdir = "data";
    Init();
  }

  void OBAtomicHeatOfFormationTable::ParseLine(const char *line)
  {
    char *ptr;
    vector<string> vs;
    OBAtomHOF *oba;

    ptr = const_cast<char*>( strchr(line,'#'));
    if (NULL != ptr)
      ptr[0] = '\0';
    if (strlen(line) > 0)
      {
        tokenize(vs,line,"|");
        if (vs.size() >= 8)
          {
              oba = new OBAtomHOF(vs[0],
                                  atoi(vs[1].c_str()),
                                  vs[2],
                                  vs[3],
                                  atof(vs[4].c_str()),
                                  atof(vs[5].c_str()),
                                  atoi(vs[6].c_str()),
                                  vs[7]);
            _atomhof.push_back(*oba);
          }
      }
  }

  int OBAtomicHeatOfFormationTable::GetHeatOfFormation(std::string elem,
                                                       int charge,
                                                       std::string meth,
                                                       double T,
                                                       double *dhof0,
                                                       double *dhofT,
                                                       double *S0T)
  {
    int    found;
    double Ttol = 0.05; /* Kelvin */
    double Vmodel, Vdhf, S0, HexpT;
    std::vector<OBAtomHOF>::iterator it;
    char desc[128];

    found = 0;
    Vmodel = Vdhf = S0 = HexpT = 0;
    snprintf(desc,sizeof(desc),"%s(0K)",meth.c_str());

    for(it = _atomhof.begin(); it != _atomhof.end(); ++it)
    {
        if ((0 == it->Element().compare(elem)) &&
            (it->Charge() == charge))
        {
            double eFac = energyToKcal(it->Unit());
            if (fabs(T - it->T()) < Ttol)
            {
                if (0 == it->Method().compare("exp"))
                {
                    if (0 == it->Desc().compare("H(0)-H(T)"))
                    {
                        HexpT += it->Value()*eFac;
                        found++;
                    }
                    else if (0 == it->Desc().compare("S0(T)"))
                    {
                        S0 += it->Value();
                        found++;
                    }
                }
            }
            else if (0 == it->T()) 
            {
                if ((0 == it->Method().compare(meth)) &&
                    (0 == it->Desc().compare(desc)))
                {
                    Vmodel += it->Value()*eFac;
                    found++;
                }
                if (0 == it->Method().compare("exp"))
                {
                    if (0 == it->Desc().compare("DHf(T)"))
                    {
                        Vdhf += it->Value()*eFac;
                        found++;
                    }
                }
            }
        }
    }

    if (found == 4)
    {
        *dhof0 = Vdhf-Vmodel;
        *dhofT = Vdhf-Vmodel-HexpT;
        *S0T   = -S0/4.184;
        return 1;
    }
    return 0;
  }

  /** \class OBTypeTable data.h <openbabel/data.h>
      \brief Atom Type Translation Table

      Molecular file formats frequently store information about atoms in an
      atom type field. Some formats store only the element for each atom,
      while others include hybridization and local environments, such as the
      Sybyl mol2 atom type field. The OBTypeTable class acts as a translation
      table to convert atom types between a number of different molecular
      file formats. The constructor for OBTypeTable automatically reads the
      text file types.txt. Just as OBElementTable, an instance of
      OBTypeTable (ttab) is declared external in data.cpp and is referenced as
      extern OBTypeTable ttab in mol.h.  The following code demonstrates how
      to use the OBTypeTable class to translate the internal representation
      of atom types in an OBMol Internal to Sybyl Mol2 atom types.

      \code
      ttab.SetFromType("INT");
      ttab.SetToType("SYB");
      OBAtom *atom;
      vector<OBAtom*>::iterator i;
      string src,dst;
      for (atom = mol.BeginAtom(i);atom;atom = mol.EndAtom(i))
      {
         src = atom->GetType();
         ttab.Translate(dst,src);
         cout << "atom number " << atom->GetIdx() << "has mol2 type " << dst << endl;
      }
      \endcode

      Current atom types include (defined in the top line of the data file types.txt):
      - INT (Open Babel internal codes)
      - ATN (atomic numbers)
      - HYB (hybridization)
      - MMD (MacroModel)
      - MM2 (MM2 force field)
      - XYZ (element symbols from XYZ file format)
      - ALC (Alchemy)
      - HAD (H added)
      - MCML (MacMolecule)
      - C3D (Chem3D)
      - SYB (Sybyl mol2)
      - MOL (Sybyl mol)
      - MAP (Gasteiger partial charge types)
      - DRE (Dreiding)
      - XED (XED format)
      - DOK (Dock)
      - M3D (Molecular Arts M3D)
      - SBN (Sybyl descriptor types for MPD files)
      - PCM (PC Model)
  */

  OBTypeTable::OBTypeTable()
  {
    _init = false;
    _dir = BABEL_DATADIR;
    _envvar = "BABEL_DATADIR";
    _filename = "types.txt";
    _subdir = "data";
    _dataptr = TypesData;
    _linecount = 0;
    _from = _to = -1;
  }

  void OBTypeTable::ParseLine(const char *buffer)
  {
    if (buffer[0] == '#')
      return; // just a comment line

    if (_linecount == 0) {
      tokenize(_colnames,buffer);
      _ncols = _colnames.size();
    }
    else
      {
        vector<string> vc;
        tokenize(vc,buffer);
        if (vc.size() == (unsigned)_ncols)
          _table.push_back(vc);
        else
          {
            stringstream errorMsg;
            errorMsg << " Could not parse line in type translation table types.txt -- incorect number of columns";
            errorMsg << " found " << vc.size() << " expected " << _ncols << ".";
            obErrorLog.ThrowError(__FUNCTION__, errorMsg.str(), obInfo);
          }
      }
    _linecount++;
  }

  bool OBTypeTable::SetFromType(const char* from)
  {
    if (!_init)
      Init();

    string tmp = from;

    unsigned int i;
    for (i = 0;i < _colnames.size();++i)
      if (tmp == _colnames[i])
        {
          _from = i;
          return(true);
        }

    obErrorLog.ThrowError(__FUNCTION__, "Requested type column not found", obInfo);

    return(false);
  }

  bool OBTypeTable::SetToType(const char* to)
  {
    if (!_init)
      Init();

    string tmp = to;

    unsigned int i;
    for (i = 0;i < _colnames.size();++i)
      if (tmp == _colnames[i])
        {
          _to = i;
          return(true);
        }

    obErrorLog.ThrowError(__FUNCTION__, "Requested type column not found", obInfo);

    return(false);
  }

  //! Translates atom types (to, from), checking for size of destination
  //!  string and null-terminating as needed
  //! \deprecated Because there is no guarantee on the length of an atom type
  //!  you should consider using std::string instead
  bool OBTypeTable::Translate(char *to, const char *from)
  {
    if (!_init)
      Init();

    bool rval;
    string sto,sfrom;
    sfrom = from;
    rval = Translate(sto,sfrom);
    strncpy(to,(char*)sto.c_str(), OBATOM_TYPE_LEN - 1);
    to[OBATOM_TYPE_LEN - 1] = '\0';

    return(rval);
  }

  bool OBTypeTable::Translate(string &to, const string &from)
  {
    if (!_init)
      Init();

    if (from == "")
      return(false);

    if (_from >= 0 && _to >= 0 &&
        _from < (signed)_table.size() && _to < (signed)_table.size())
      {
        vector<vector<string> >::iterator i;
        for (i = _table.begin();i != _table.end();++i)
          if ((signed)(*i).size() > _from &&  (*i)[_from] == from)
            {
              to = (*i)[_to];
              return(true);
            }
      }

    // Throw an error, copy the string and return false
    obErrorLog.ThrowError(__FUNCTION__, "Cannot perform atom type translation: table cannot find requested types.", obWarning);
    to = from;
    return(false);
  }

  std::string OBTypeTable::Translate(const string &from)
  {
    if (!_init)
      Init();

    if (from.empty())
      return("");

    if (_from >= 0 && _to >= 0 &&
        _from < (signed)_table.size() && _to < (signed)_table.size())
      {
        vector<vector<string> >::iterator i;
        for (i = _table.begin();i != _table.end();++i)
          if ((signed)(*i).size() > _from &&  (*i)[_from] == from)
            {
              return (*i)[_to];
            }
      }

    // Throw an error, copy the string and return false
    obErrorLog.ThrowError(__FUNCTION__, "Cannot perform atom type translation: table cannot find requested types.", obWarning);
    return("");
  }

  std::string OBTypeTable::GetFromType()
  {
    if (!_init)
      Init();

    if (_from > 0 && _from < (signed)_table.size())
      return( _colnames[_from] );
    else
      return( _colnames[0] );
  }

  std::string OBTypeTable::GetToType()
  {
    if (!_init)
      Init();

    if (_to > 0 && _to < (signed)_table.size())
      return( _colnames[_to] );
    else
      return( _colnames[0] );
  }

  void Toupper(string &s)
  {
    unsigned int i;
    for (i = 0;i < s.size();++i)
      s[i] = toupper(s[i]);
  }

  void Tolower(string &s)
  {
    unsigned int i;
    for (i = 0;i < s.size();++i)
      s[i] = tolower(s[i]);
  }

  ///////////////////////////////////////////////////////////////////////
  OBResidueData::OBResidueData()
  {
    _init = false;
    _dir = BABEL_DATADIR;
    _envvar = "BABEL_DATADIR";
    _filename = "resdata.txt";
    _subdir = "data";
    _dataptr = ResidueData;
  }

  bool OBResidueData::AssignBonds(OBMol &mol,OBBitVec &bv)
  {
    if (!_init)
      Init();

    OBAtom *a1,*a2;
    OBResidue *r1,*r2;
    vector<OBAtom*>::iterator i,j;
    vector3 v;

    int bo;
    string skipres = ""; // Residue Number to skip
    string rname = "";
    //assign residue bonds
    for (a1 = mol.BeginAtom(i);a1;a1 = mol.NextAtom(i))
      {
        r1 = a1->GetResidue();
        if (r1 == NULL) // atoms may not have residues
          continue;

        if (skipres.length() && r1->GetNumString() == skipres)
          continue;

        if (r1->GetName() != rname)
          {
            skipres = SetResName(r1->GetName()) ? "" : r1->GetNumString();
            rname = r1->GetName();
          }
        //assign bonds for each atom
        for (j=i,a2 = mol.NextAtom(j);a2;a2 = mol.NextAtom(j))
          {
            r2 = a2->GetResidue();
            if (r2 == NULL) // atoms may not have residues
              continue;

            if (r1->GetNumString() != r2->GetNumString())
              break;
            if (r1->GetName() != r2->GetName())
              break;
            if (r1->GetChain() != r2->GetChain())
              break; // Fixes PR#2889763 - Fabian

            if ((bo = LookupBO(r1->GetAtomID(a1),r2->GetAtomID(a2))))
              {
                // Suggested by Liu Zhiguo 2007-08-13
                // for predefined residues, don't perceive connection
                // by distance
                //                v = a1->GetVector() - a2->GetVector();
                //                if (v.length_2() < 3.5) //check by distance
                  mol.AddBond(a1->GetIdx(),a2->GetIdx(),bo);
              }
          }
      }

    int hyb;
    string type;

    //types and hybridization
    rname = ""; // name of current residue
    skipres = ""; // don't skip any residues right now
    for (a1 = mol.BeginAtom(i);a1;a1 = mol.NextAtom(i))
      {
        if (a1->IsOxygen() && !a1->GetValence())
          {
            a1->SetType("O3");
            continue;
          }
        if (a1->IsHydrogen())
          {
            a1->SetType("H");
            continue;
          }

        //***valence rule for O-
        if (a1->IsOxygen() && a1->GetValence() == 1)
          {
            OBBond *bond;
            bond = (OBBond*)*(a1->BeginBonds());
            if (bond->GetBO() == 2)
              {
                a1->SetType("O2");
                a1->SetHyb(2);
              }
            else if (bond->GetBO() == 1)
              {
                // Leave the protonation/deprotonation to phmodel.txt
                a1->SetType("O3");
                a1->SetHyb(3);
                // PR#3203039 -- Fix from Magnus Lundborg
                //                a1->SetFormalCharge(0);
              }
            continue;
          }

        r1 = a1->GetResidue();
        if (r1 == NULL) continue; // atoms may not have residues
        if (skipres.length() && r1->GetNumString() == skipres)
          continue;

        if (r1->GetName() != rname)
          {
            // if SetResName fails, skip this residue
            skipres = SetResName(r1->GetName()) ? "" : r1->GetNumString();
            rname = r1->GetName();
          }

        if (LookupType(r1->GetAtomID(a1),type,hyb))
          {
            a1->SetType(type);
            a1->SetHyb(hyb);
          }
        else // try to figure it out by bond order ???
          {}
      }

    return(true);
  }

  void OBResidueData::ParseLine(const char *buffer)
  {
    int bo;
    string s;
    vector<string> vs;

    if (buffer[0] == '#')
      return;

    tokenize(vs,buffer);
    if (!vs.empty())
      {
        if (vs[0] == "BOND")
          {
            s = (vs[1] < vs[2]) ? vs[1] + " " + vs[2] :
              vs[2] + " " + vs[1];
            bo = atoi(vs[3].c_str());
            _vtmp.push_back(pair<string,int> (s,bo));
          }

        if (vs[0] == "ATOM" && vs.size() == 4)
          {
            _vatmtmp.push_back(vs[1]);
            _vatmtmp.push_back(vs[2]);
            _vatmtmp.push_back(vs[3]);
          }

        if (vs[0] == "RES")
          _resname.push_back(vs[1]);

        if (vs[0]== "END")
          {
            _resatoms.push_back(_vatmtmp);
            _resbonds.push_back(_vtmp);
            _vtmp.clear();
            _vatmtmp.clear();
          }
      }
  }

  bool OBResidueData::SetResName(const string &s)
  {
    if (!_init)
      Init();

    unsigned int i;

    for (i = 0;i < _resname.size();++i)
      if (_resname[i] == s)
        {
          _resnum = i;
          return(true);
        }

    _resnum = -1;
    return(false);
  }

  int OBResidueData::LookupBO(const string &s)
  {
    if (_resnum == -1)
      return(0);

    unsigned int i;
    for (i = 0;i < _resbonds[_resnum].size();++i)
      if (_resbonds[_resnum][i].first == s)
        return(_resbonds[_resnum][i].second);

    return(0);
  }

  int OBResidueData::LookupBO(const string &s1, const string &s2)
  {
    if (_resnum == -1)
      return(0);
    string s;

    s = (s1 < s2) ? s1 + " " + s2 : s2 + " " + s1;

    unsigned int i;
    for (i = 0;i < _resbonds[_resnum].size();++i)
      if (_resbonds[_resnum][i].first == s)
        return(_resbonds[_resnum][i].second);

    return(0);
  }

  bool OBResidueData::LookupType(const string &atmid,string &type,int &hyb)
  {
    if (_resnum == -1)
      return(false);

    string s;
    vector<string>::iterator i;

    for (i = _resatoms[_resnum].begin();i != _resatoms[_resnum].end();i+=3)
      if (atmid == *i)
        {
          ++i;
          type = *i;
          ++i;
          hyb = atoi((*i).c_str());
          return(true);
        }

    return(false);
  }

  void OBGlobalDataBase::Init()
  {
    if (_init)
      return;
    _init = true;

    ifstream ifs;
    char charBuffer[BUFF_SIZE];

    // Set the locale for number parsing to avoid locale issues: PR#1785463
    obLocale.SetLocale();

    // Check return value from OpenDatafile
    // Suggestion from Zhiguo Liu
    string fn_open = OpenDatafile(ifs, _filename, _envvar);

    if (fn_open != "" && (ifs))
      {
        while(ifs.getline(charBuffer,BUFF_SIZE))
          ParseLine(charBuffer);
      }

    else
      // If all else fails, use the compiled in values
      if (_dataptr)
        {
          obErrorLog.ThrowError(__FUNCTION__, "Cannot open " + _filename + " defaulting to compiled data.", obWarning);

          const char *p1,*p2;
          for (p1 = p2 = _dataptr;*p2 != '\0';++p2)
            if (*p2 == '\n')
              {
                strncpy(charBuffer, p1, (p2 - p1));
                charBuffer[(p2 - p1)] = '\0';
                ParseLine(charBuffer);
                p1 = ++p2;
              }
        }
      else
        {
          string s = "Unable to open data file '";
          s += _filename;
          s += "'";
          obErrorLog.ThrowError(__FUNCTION__, s, obWarning);
        }

    // return the locale to the original one
    obLocale.RestoreLocale();

    if (ifs)
      ifs.close();

    if (GetSize() == 0)
      {
        string s = "Cannot initialize database '";
        s += _filename;
        s += "' which may cause further errors.";
        obErrorLog.ThrowError(__FUNCTION__, s, obWarning);
      }

  }

} // end namespace OpenBabel

double energyToKcal(std::string unit)
{
    if ((unit.compare("kJ/mol") == 0) ||
        (unit.compare("J/mol K") == 0))
    {
        return 1.0/4.184;
    }
    else if (unit.compare("Hartree") == 0)
    {
        return 627.509469;
    }
    else if (unit.compare("Rydberg") == 0)
    {
        return 313.755026;
    }
    else if ((unit.compare("eV") == 0) ||
             (unit.compare("electronvolt") == 0))
    {
        return 23.060538;
    }
    else if (unit.compare("kcal/mol") == 0)
    {
        return 1;
    }
    fprintf(stderr, "Unknown energy unit %s in file %s, line %d\n", 
            unit.c_str(), __FILE__, __LINE__);
    
    return 1;
}

bool extract_thermochemistry(OpenBabel::OBMol  &mol,
                             bool    bVerbose,
                             int    *Nsymm,
                             int     Nrotbonds,
                             double  dBdT,
                             double *temperature,
                             double *DeltaHf0,
                             double *DeltaHfT,
                             double *DeltaGfT,
                             double *DeltaSfT,
                             double *S0T,
                             double *CVT,
                             double *CPT,
                             std::vector<double> &Scomponents)
{
    enum kkTYPE { kkDH, kkDG, kkDS, kkS0, kkCV, kkSt, kkSr, kkSv };
    typedef struct {
        string term;
        kkTYPE kk;
    } energy_unit;
    double St = 0, Sr = 0, Sv = 0, Sconf = 0, Ssymm = 0;
    double Rgas      = 1.9872041; 
    int    RotSymNum = 1;
    OpenBabel::OBRotationData* rd;
    
    rd = (OpenBabel::OBRotationData*)mol.GetData("RotationData");
    if (NULL != rd)
    {
        RotSymNum = rd->GetSymmetryNumber();
        if (bVerbose)
        {
            printf("Found symmetry number %d in input file.\n", RotSymNum);
        }
    }
    else if (bVerbose)
    {
        printf("Using default symmetry number %d\n", RotSymNum);
    }
    if ((*Nsymm > 0) && (*Nsymm != RotSymNum))
    {
        // Rgas in cal/mol K http://en.wikipedia.org/wiki/Gas_constant
        Ssymm = -Rgas*log((1.0* *Nsymm)/RotSymNum);
        RotSymNum = *Nsymm;
        if (bVerbose)
        {
            printf("Changing symmetry number to %d\n", RotSymNum);
        }
    }
    else if (*Nsymm == 0)
    {
        *Nsymm = RotSymNum;
    }
    if (Nrotbonds > 0) 
    {
        Sconf = Rgas*Nrotbonds*log(3.0);
    }
    energy_unit eu[] = {
        { "DeltaHform", kkDH },
        { "DeltaGform", kkDG },
        { "DeltaSform", kkDS },
        { "S0",         kkS0 },
        { "cv",         kkCV },
        { "Strans",     kkSt },
        { "Srot",       kkSr },
        { "Svib",       kkSv }
    };
#define NEU (sizeof(eu)/sizeof(eu[0]))
    int found = 0;
    std::vector<OpenBabel::OBGenericData*> obdata = mol.GetData();
    for(std::vector<OpenBabel::OBGenericData*>::iterator j = obdata.begin(); (j<obdata.end()); ++j)
    {
        string term  = (*j)->GetAttribute();
        double value = atof((*j)->GetValue().c_str());
        double T     = 0;
        {
            size_t lh = term.find("(");
            size_t rh = term.find("K)");
            double TT = atof(term.substr(lh+1,rh-lh-1).c_str());
            if (0 != TT)
            {
                if (0 == T)
                {
                    T            = TT;
                    *temperature = TT;
                }
                else
                {
                    cerr << "Different T in the input file, found " << T << " before and now " << TT << ". Output maybe inconsistent.";
                    T = TT;
                }
            }
        }
        for(int i = 0; (i<NEU); i++)
        {
            if (strstr(term.c_str(), eu[i].term.c_str()) != 0)
            {
                switch (eu[i].kk)
                {
                case kkDH:
                    if (0 == T)
                    {
                        *DeltaHf0 = value;
                    }
                    else
                    {
                        *DeltaHfT = value;
                    }
                    found ++;
                    break;
                case kkDG:
                    *DeltaGfT = value - T*(Ssymm+Sconf)/1000;
                    found ++;
                    break;
                case kkDS:
                    *DeltaSfT = value + Ssymm + Sconf;
                    found ++;
                    break;
                case kkS0:
                    *S0T = value + Ssymm + Sconf;
                    found ++;
                    break;
                case kkSt:
                    St = value;
                    found ++;
                    break;
                case kkSr:
                    Sr = value;
                    found ++;
                    break;
                case kkSv:
                    Sv = value;
                    found ++;
                    break;
                case kkCV:
                    *CVT = value;
                    found++;
                    break;
                default:
                    break;
                }
            }
        }
    }
    double P   = 16.605/4.184; // Convert pressure to kcal/mol
    *CPT       = *CVT + Rgas + (2*P*dBdT + pow(P*dBdT, 2.0)/Rgas);

    Scomponents.push_back(St);
    Scomponents.push_back(Sr);
    Scomponents.push_back(Sv);
    Scomponents.push_back(Ssymm);
    Scomponents.push_back(Sconf);
    if (bVerbose && (Ssymm != 0))
    {
        printf("Applyied symmetry correction to free energy of %g kcal/mol\n",
               -(*temperature*Ssymm)/1000);
    }
    if (bVerbose && (Sconf != 0))
    {
        printf("Applyied conformational correction to free energy of %g kcal/mol\n",
               -(*temperature*Sconf)/1000);
    }
    return (found == 9);
}

//! \file data.cpp
//! \brief Global data and resource file parsers.
