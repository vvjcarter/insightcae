#ifndef INSIGHT_INTERFOAMNUMERICS_H
#define INSIGHT_INTERFOAMNUMERICS_H

#include "openfoam/caseelements/numerics/fvnumerics.h"
#include "openfoam/caseelements/numerics/pimplesettings.h"
#include "interfoamnumerics__interFoamNumerics__Parameters_headers.h"

namespace insight {


OFDictData::dict stdMULESSolverSetup
(
        double cAlpha,
        double icAlpha,
        double tol=1e-12,
        double reltol=0.0,
        bool LTS=false,
        int nLimiterIter=15
);



class interFoamNumerics
    : public FVNumerics
{

public:
#include "interfoamnumerics__interFoamNumerics__Parameters.h"
/*
PARAMETERSET>>> interFoamNumerics Parameters
inherits FVNumerics::Parameters

alphainternal = double 0.0 "Internal phase fraction field value"
pinternal = double 0.0 "Internal pressure field value"
Uinternal = vector (0 0 0) "Internal velocity field value"

time_integration = includedset "insight::MultiphasePIMPLESettings::Parameters" "Settings for time integration"


alphaSubCycles = int 1 "Number of alpha integration subcycles"
alphaLimiterIter = int 15 "Number of alpha limiter iterations"

cAlpha = double 0.25 "[-] Interface compression coefficient"
icAlpha = double 0 "[-] Isotropic interface compression coefficient"

snGradLowQualityLimiterReduction = double 0.66 "Reduction of limiter coefficient on low quality faces"

<<<PARAMETERSET
*/

protected:
    Parameters p_;
    std::string alphaname_;


public:
    declareType ( "interFoamNumerics" );
    interFoamNumerics ( OpenFOAMCase& c, const ParameterSet& ps = Parameters::makeDefault() );
    void addIntoDictionaries ( OFdicts& dictionaries ) const override;
    bool isCompressible() const override;

    inline const std::string& pressureFieldName() const
    {
        return pName_;
    }
    inline const std::string& alphaFieldName() const
    {
        return alphaname_;
    }
};

} // namespace insight

#endif // INSIGHT_INTERFOAMNUMERICS_H
