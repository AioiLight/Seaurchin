#pragma once

namespace sefx
{
    using namespace boost::spirit;
    using boost::optional;
    using namespace std;

    struct EffectDistribution
    {
        string name;
        vector<double> parameters;
    };

    template<typename T>
    struct EffectParameter
    {
        string name;
        vector<T> values;
    };

    typedef tuple<
        optional<EffectParameter<EffectDistribution>>,
        optional<EffectParameter<string>>,
        optional<EffectParameter<double>>
    > EffectOptionals;

    typedef tuple<string, vector<tuple<
        optional<EffectParameter<EffectDistribution>>,
        optional<EffectParameter<string>>,
        optional<EffectParameter<double>>,
        optional<vector<tuple<EffectOptionals>>>>>>
        EffectTuple;

    template<typename Iter>
    struct EffectGrammar
        : qi::grammar<Iter, EffectTuple(), qi::space_type>
    {
        //���ە�����̎����̓p�N
        qi::symbols<char const, char const> unesc_char;                                         //������p�G�X�P�Z�b�g
        qi::rule<Iter, string()> rString;                                                       //������
        qi::rule<Iter, string()> rIdentifer;                                                    //���ʎq
        qi::rule<Iter, EffectDistribution(), qi::space_type> rDist;                             //���z����
        //���v�f
        qi::rule<Iter, string(), qi::space_type> rType;                                         //�^�C�v�錾
        qi::rule<Iter, EffectParameter<EffectDistribution>(), qi::space_type> rDistParam;       //���z�p�����[�^�[
        qi::rule<Iter, EffectParameter<string>(), qi::space_type> rStringParam;                 //������p�����[�^�[
        qi::rule<Iter, EffectParameter<double>(), qi::space_type> rNumberParam;                 //���l�p�����[�^�[
        qi::rule<Iter, vector<EffectOptionals>(), qi::space_type> rEmitter;                     //�G�~�b�^
        qi::rule<Iter, EffectTuple(), qi::space_type> rEffect;                                  //�G�t�F�N�g

        //�\���v�f
        qi::rule<Iter, qi::space_type> rStatement;

        EffectGrammar() : EffectGrammar::base_type(rEffect)
        {
            rString = '"' >> *(unesc_char | qi::alnum | "\\x" >> qi::hex) >> '"';
            unesc_char.add
            ("\\a", '\a')("\\b", '\b')("\\f", '\f')("\\n", '\n')
                ("\\r", '\r')("\\t", '\t')("\\v", '\v')("\\\\", '\\')
                ("\\\'", '\'')("\\\"", '\"');
            rIdentifer = qi::alpha >> *qi::alnum;
            rDist = rIdentifer >> '(' >> (qi::double_ % ',') >> ')';

            rType = qi::lit("type") >> rIdentifer >> ';';
            rDistParam = rIdentifer >> (rDist % ',') >> ';';
            rStringParam = rIdentifer >> (rString % ',') >> ';';
            rNumberParam = rIdentifer >> (qi::double_ % ',') >> ';';

            rEmitter = qi::lit("emitter") >> '{'
                >> *(rDistParam ^ rStringParam ^ rNumberParam)
                >> '}';

            rEffect = qi::lit("effect") >> rIdentifer >> '{'
                >> *(rDistParam ^ rStringParam ^ rNumberParam ^ rEmitter)
                >> '}';
        }
    };
}

BOOST_FUSION_ADAPT_STRUCT(
    sefx::EffectDistribution,
    (std::string, name)
    (std::vector<double>, parameters)
)

BOOST_FUSION_ADAPT_STRUCT(
    sefx::EffectParameter<sefx::EffectDistribution>,
    (std::string, name)
    (std::vector<sefx::EffectDistribution>, values)
)

BOOST_FUSION_ADAPT_STRUCT(
    sefx::EffectParameter<std::string>,
    (std::string, name)
    (std::vector<std::string>, values)
)

BOOST_FUSION_ADAPT_STRUCT(
    sefx::EffectParameter<double>,
    (std::string, name)
    (std::vector<double>, values)
)