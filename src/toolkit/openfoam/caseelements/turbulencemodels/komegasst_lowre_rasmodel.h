#ifndef INSIGHT_KOMEGASST_LOWRE_RASMODEL_H
#define INSIGHT_KOMEGASST_LOWRE_RASMODEL_H

#include "komegasst_rasmodel.h"

namespace insight {

class kOmegaSST_LowRe_RASModel
: public kOmegaSST_RASModel
{
public:
  declareType("kOmegaSST_LowRe");

  kOmegaSST_LowRe_RASModel(OpenFOAMCase& c, const ParameterSet& ps = ParameterSet());
  void addIntoDictionaries(OFdicts& dictionaries) const override;
  bool addIntoFieldDictionary(const std::string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, double roughness_z0) const override;
  inline static ParameterSet defaultParameters() { return ParameterSet(); }
};

} // namespace insight

#endif // INSIGHT_KOMEGASST_LOWRE_RASMODEL_H
