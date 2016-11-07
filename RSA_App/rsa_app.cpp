// Copyright (c) 2015-2016 Bitdefender SRL, All rights reserved.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3.0 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library.

#include <bdvmi/backendfactory.h>
#include <bdvmi/domainhandler.h>
#include <bdvmi/domainwatcher.h>
#include <bdvmi/eventhandler.h>
#include <bdvmi/eventmanager.h>
#include <bdvmi/loghelper.h>
#include <iostream>
#include <memory>
#include <signal.h>
#include <sstream>

#define DEBUG 1

using namespace std;

namespace { // Anonymous namespace

sig_atomic_t stop;

void stop_handler( int /* signo */ )
{
	stop = 1;
}

}

class RSALogHelper : public bdvmi::LogHelper {

public:
	RSALogHelper( const string &domainName = "" )
	{
		if ( !domainName.empty() )
			prefix_ = string( "[" ) + domainName + "] ";
	}

public:
	virtual void error( const string &message )
	{
		cerr << prefix_ << "[ERROR] " << message << endl;
	}

	virtual void warning( const string &message )
	{
		cout << prefix_ << "[WARNING] " << message << endl;
	}

	virtual void info( const string &message )
	{
		cout << prefix_ << "[INFO] " << message << endl;
	}

	virtual void debug( const string &message )
	{
		cout << prefix_ << "[DEBUG] " << message << endl;
	}

private:
	string prefix_;
};

class RSAEventHandler : public bdvmi::EventHandler {

public:

	// Callback for page faults
	virtual void handlePageFault( unsigned short vcpu, const bdvmi::Registers & /* regs */,
	                              uint64_t physAddress, uint64_t virtAddress, bool /* read */,
	                              bool /* write */, bool /* execute */, bdvmi::HVAction & /* action */,
	                              uint8_t * /* data */, uint32_t & /* size */,
	                              unsigned short & /* instructionLength */ )
	{
		cout << "Page fault event on VCPU: " << vcpu << endl;
		cout << "\tPhysAddress: " << hex << physAddress << endl;
		cout << "\tVirtAddress: " << hex << virtAddress << endl;
	}

	virtual void handleVMCALL( unsigned short vcpu, const bdvmi::Registers & /* regs */, uint64_t /* rip */,
	                           uint64_t eax )
	{
		cout << "VMCALL event on VCPU " << vcpu << ", EAX: 0x" << hex << eax << endl;
	}

	virtual void handleSessionOver( bool /* domainStillRunning */ )
	{
		cout << "Session over." << endl;
	}

	// This callback will run before each event (helper)
	virtual void runPreEvent()
	{
		cout << "Prepare for event ..." << endl;
	}

	virtual void handleFatalError()
	{
		throw std::runtime_error( "A fatal error occurred, cannot continue" );
	}

	virtual void runPostEvent()
	{
		cout << "Event handled ..." << endl;
	}

};

class RSADomainHandler : public bdvmi::DomainHandler {

public:
	RSADomainHandler( bdvmi::BackendFactory &bf ) : bf_( bf )
	{
	}

public:
	// Found a domain
	virtual void handleDomainFound( const string &domain )
	{
		cout << "A new domain started running: " << domain << endl;
		hookDomain( domain );
	}

	// The domain is no longer running
	virtual void handleDomainFinished( const string &domain )
	{
		cout << "Domain finished: " << domain << endl;
	}

	virtual void cleanup()
	{
		cout << "Done waiting for domains to start." << endl;
	}

private:
	void hookDomain( const string &domain )
	{
		auto_ptr<bdvmi::Driver> pd( bf_.driver( domain ) );
		auto_ptr<bdvmi::EventManager> em( bf_.eventManager( *pd, bdvmi::EventManager::ENABLE_MEMORY ) );

        unsigned long long start;

        cout << "Input start: ";
        cin >> hex >> start;

#ifdef DEBUG
        unsigned long gfn = ( (unsigned long)(start >> 12) );
        cout << "gfn: " << gfn << endl;
        cout << "gfn hex: " << hex << gfn << endl;

        bool r = false, w = false, x = false;

        if (r) cout << "r true" << endl;
        if (w) cout << "w true" << endl;
        if (x) cout << "x true" << endl;

        cout << "getting page protection" << endl;

        pd->getPageProtection( start, r, w, x  );

        if (r) cout << "r true" << endl;
        if (w) cout << "w true" << endl;
        if (x) cout << "x true" << endl;
#endif

        cout << "setting page protection" << endl;

        for (int i = 0; i < 4000; i++)
        {
            pd->setPageProtection( (start+i), false, false, false );
        }

#ifdef DEBUG
        pd->getPageProtection( start, r, w, x  );

        if (r) cout << "r true" << endl;
        if (w) cout << "w true" << endl;
        if (x) cout << "x true" << endl;
#endif

		RSAEventHandler reh;

		em->signalStopVar( &stop );

		em->handler( &reh );

		em->waitForEvents();
	}

private:
	bdvmi::BackendFactory &bf_;
};

int main()
{
	try {
		signal( SIGINT, stop_handler );
		signal( SIGHUP, stop_handler );
		signal( SIGTERM, stop_handler );

		RSALogHelper logHelper;
		bdvmi::BackendFactory bf( bdvmi::BackendFactory::BACKEND_XEN, &logHelper );
		RSADomainHandler rdh( bf );

		// Unique_ptr<T> would have been better, but the user's compiler might not
		// support C++0x.
		auto_ptr<bdvmi::DomainWatcher> pdw( bf.domainWatcher() );

		cout << "Registering handler ... " << endl;

		pdw->handler( &rdh );

		cout << "Setting up break-out-of-the-loop (stop) variable ..." << endl;
		pdw->signalStopVar( &stop );

		cout << "Waiting for domains ..." << endl;
		pdw->waitForDomains();

		cout << "\nDone." << endl;

	} catch ( const exception &e ) {
		cerr << "Error: caught exception: " << e.what() << endl;
		return -1;
	}

	return 0;
}
