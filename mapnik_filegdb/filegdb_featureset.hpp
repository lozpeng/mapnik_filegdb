#ifndef FILEGDB_FEATURESET_HPP
#define FILEGDB_FEATURESET_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>

// boost
#include <boost/scoped_ptr.hpp> // needed for wrapping the transcoder

class filegdb_featureset : public mapnik::Featureset
{
public:
    // this constructor can have any arguments you need
    filegdb_featureset(mapnik::box2d<double> const& box, std::string const& encoding);

    // desctructor
    virtual ~filegdb_featureset();

    // mandatory: you must expose a next() method, called when rendering
    mapnik::feature_ptr next();

private:
    // members are up to you, but these are recommended
    mapnik::box2d<double> box_;
    mapnik::value_integer feature_id_;
    boost::scoped_ptr<mapnik::transcoder> tr_;
    mapnik::context_ptr ctx_;
};

#endif // FILEGDB_FEATURESET_HPP
