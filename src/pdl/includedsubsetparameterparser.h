#ifndef INCLUDEDSUBSETPARAMETERPARSER_H
#define INCLUDEDSUBSETPARAMETERPARSER_H


#include "parserdatabase.h"


struct IncludedSubsetParameterParser
{

    struct Data
            : public ParserDataBase
    {
        std::string value;

        typedef boost::fusion::vector3<std::string, std::string, std::string> DefaultModification;
        typedef std::vector<DefaultModification> DefaultValueModifications;

        DefaultValueModifications default_value_modifications;

        Data(const std::string& v, const std::string& d, const DefaultValueModifications& defmod);

        void cppAddHeader(std::set<std::string>& headers) const override;

        std::string cppType(const std::string&) const override;

        std::string cppValueRep(const std::string&, const std::string& thisscope ) const override;

        std::string cppParamType(const std::string& ) const override;

        void cppWriteCreateStatement
        (
            std::ostream& os,
            const std::string& name,
            const std::string& thisscope
        ) const override;

        void cppWriteInsertStatement
        (
            std::ostream& os,
            const std::string& psvarname,
            const std::string& name,
            const std::string& thisscope
        ) const override;


        void cppWriteSetStatement
        (
            std::ostream& os,
            const std::string& ,
            const std::string& varname,
            const std::string& staticname,
            const std::string&
        ) const override;

        void cppWriteGetStatement
        (
            std::ostream& os,
            const std::string& ,
            const std::string& varname,
            const std::string& staticname,
            const std::string&
        ) const override;

    };

    template <typename Iterator, typename Skipper = skip_grammar<Iterator> >
    inline static void insertrule(PDLParserRuleset<Iterator,Skipper>& ruleset)
    {
        ruleset.parameterDataRules.add
        (
            "includedset",
            typename PDLParserRuleset<Iterator,Skipper>::ParameterDataRulePtr(
                      new typename PDLParserRuleset<Iterator,Skipper>::ParameterDataRule(
                        (ruleset.r_string >> ruleset.r_description_string >>
                         (
                           (qi::lit("modifyDefaults") >> '{' >>
                            *(
                              ruleset.r_identifier >> ruleset.r_path
                                >> '=' >> ruleset.r_up_to_semicolon
                             )
                           >> '}')
                           |qi::attr(Data::DefaultValueModifications())
                          ) )
                        [ qi::_val = phx::construct<ParserDataBase::Ptr>(phx::new_<Data>(qi::_1, qi::_2, qi::_3)) ]
                    ))
        );
    }
};


#endif // INCLUDEDSUBSETPARAMETERPARSER_H
