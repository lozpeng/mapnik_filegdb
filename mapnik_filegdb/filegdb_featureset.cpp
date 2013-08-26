#include <iostream>

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/make_shared.hpp>

// file gdb plugin
#include "filegdb_featureset.hpp"
#include "shape_buffer_io.hpp"

using mapnik::geometry_type;
using mapnik::feature_factory;
using mapnik::context_ptr;

template <typename filterT>
filegdb_featureset<filterT>::filegdb_featureset(filterT const& filter,
				box2d<double> const& box,Table* gdbTable,int row_limit,
				std::string const& encoding,std::string where_clause)
		:filter_(filter),
		row_limit_(row_limit),
		box_(box),
		ctx_(boost::make_shared<mapnik::context_type>()),
		tr_(new transcoder(encoding)),
		where_clause_(where_clause)
{
	FileGDBAPI::Envelope envelope;
	envelope.xMin =box_.minx();// -118.219;
	envelope.yMin =box_.miny();//  22.98;
	envelope.xMax =box_.maxx();// -117.988;
	envelope.yMax =box_.maxy();//  34.0;
	fgdbError hr;
	//���������������������ڻ�ȡÿһ����¼
	try
	{
		if(!where_clause_.empty())
		{
			std::wstring w_clause;
			std::string str(where_clause_);
			str = gbk_to_utf8(str);
			w_clause = to_vwstring(w_clause,str);
			//where ������õ���ΪUTF-8�����ʽ
			hr = gdbTable->Search(L"*", w_clause, envelope, true, spQueryRows);
			if (hr != S_OK)
			{
				//���ݲ�ѯʧ��
			}
		}
		else{
			hr = gdbTable->Search(L"*", L"", envelope, true, spQueryRows);
		}
		if (hr != S_OK)
		{
			//spQueryRows =NULL;	
		}
	}
	catch(...)
	{
		//spQueryRows = NULL;
	}
}
template <typename filterT>
filegdb_featureset<filterT>::~filegdb_featureset() {
	try{
		spQueryRows.Close();
	}
	catch(...)
	{
	
	}
}

//
template <typename filterT>
mapnik::feature_ptr filegdb_featureset<filterT>::next()
{
	//filter_in_box filter(box_);
	//����в�ѯ�����м�
	fgdbError hr;
	Row spQueryRow;
	hr = spQueryRows.Next(spQueryRow);
	if(hr==S_OK)
	{
		int32 oid;
		hr = spQueryRow.GetOID(oid);
		if(hr!=S_OK)
			return mapnik::feature_ptr();
		mapnik::value_integer fid(oid);
		feature_ptr feature(feature_factory::create(ctx_,fid));
		shape_buffer_io::parse_esri_row(feature,&spQueryRow,*tr_);
		return feature;
	}
	// otherwise return an empty feature
	return mapnik::feature_ptr();
}

//��������Χ��ѯģ����
template class filegdb_featureset<mapnik::filter_in_box>;
//������λ�ò�ѯģ����
template class filegdb_featureset<mapnik::filter_at_point>;
