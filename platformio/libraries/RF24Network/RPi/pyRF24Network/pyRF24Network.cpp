#include "boost/python.hpp"

#include "RF24Network/RF24Network.h"
#include "RF24/RF24.h"

namespace bp = boost::python;

// **************** expicit wrappers *****************
// where needed, especially where buffer is involved
//
bp::tuple read_wrap(RF24Network& ref, size_t maxlen)
{
	size_t len;
        char *buf = new char[maxlen+1];
	RF24NetworkHeader header;

        len=ref.read(header, buf, maxlen);
        std::string str(buf, len);
	delete[] buf;
	return bp::make_tuple(header, str);
}

bool write_wrap(RF24Network& ref, RF24NetworkHeader& header, std::string message)
{
	return ref.write(header, message.c_str(), message.length());
}

std::string toString_wrap(RF24NetworkHeader& ref)
{
	return std::string(ref.toString());
}


// **************** RF24Network exposed  *****************
//
BOOST_PYTHON_MODULE(RF24Network){
    { //::RF24Network
        typedef bp::class_< RF24Network > RF24Network_exposer_t;
        RF24Network_exposer_t RF24Network_exposer = RF24Network_exposer_t( "RF24Network", bp::init< RF24 & >(( bp::arg("_radio") )) );
        bp::scope RF24Network_scope( RF24Network_exposer );
        bp::implicitly_convertible< RF24 &, RF24Network >();
        { //::RF24Network::available
        
            typedef bool ( ::RF24Network::*available_function_type )(  ) ;
            
            RF24Network_exposer.def( 
                "available"
                , available_function_type( &::RF24Network::available ) );
        
        }
        { //::RF24Network::begin
        
            typedef void ( ::RF24Network::*begin_function_type )( ::uint8_t,::uint16_t ) ;
            
            RF24Network_exposer.def( 
                "begin"
                , begin_function_type( &::RF24Network::begin )
                , ( bp::arg("_channel"), bp::arg("_node_address") ) );
        
        }
        { //::RF24Network::parent
        
            typedef ::uint16_t ( ::RF24Network::*parent_function_type )(  ) const;
            
            RF24Network_exposer.def( 
                "parent"
                , parent_function_type( &::RF24Network::parent ) );
        
        }
        { //::RF24Network::peek
        
            typedef void ( ::RF24Network::*peek_function_type )( ::RF24NetworkHeader & ) ;
            
            RF24Network_exposer.def( 
                "peek"
                , peek_function_type( &::RF24Network::peek )
                , ( bp::arg("header") ) );
        
        }
        { //::RF24Network::read
        
            typedef bp::tuple ( *read_function_type )(::RF24Network&, size_t ) ;
            
            RF24Network_exposer.def( 
                "read"
                //, read_function_type( &::RF24Network::read )
                , read_function_type( &read_wrap )
                , ( bp::arg("maxlen") ) );
        
        }
        { //::RF24Network::update
        
            typedef void ( ::RF24Network::*update_function_type )(  ) ;
            
            RF24Network_exposer.def( 
                "update"
                , update_function_type( &::RF24Network::update ) );
        
        }
        { //::RF24Network::write
        
            typedef bool ( *write_function_type )( ::RF24Network&, ::RF24NetworkHeader&, std::string ) ;
            
            RF24Network_exposer.def("write", write_function_type( &write_wrap ), ( bp::arg("header"), bp::arg("message") ) );
        
        }
        RF24Network_exposer.def_readwrite( "txTimeout", &RF24Network::txTimeout );
    }

// **************** RF24NetworkHeader exposed  *****************
//
    bp::class_< RF24NetworkHeader >( "RF24NetworkHeader", bp::init< >() )    
        .def( bp::init< uint16_t, bp::optional< unsigned char > >(( bp::arg("_to"), bp::arg("_type")=(unsigned char)(0) )) )    
        .def("toString", &toString_wrap )    
        .def_readwrite( "from_node", &RF24NetworkHeader::from_node )    
        .def_readwrite( "id", &RF24NetworkHeader::id )    
        .def_readwrite( "next_id", RF24NetworkHeader::next_id )    
        .def_readwrite( "reserved", &RF24NetworkHeader::reserved )    
        .def_readwrite( "to_node", &RF24NetworkHeader::to_node )    
        .def_readwrite( "type", &RF24NetworkHeader::type );
}

