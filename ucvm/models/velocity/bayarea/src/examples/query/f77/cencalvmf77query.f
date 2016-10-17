c -*- Fortran -*-
c
c ======================================================================
c
c                           Brad T. Aagaard
c                        U.S. Geological Survey
c
c {LicenseText}
c
c ======================================================================
c

c Application demonstrating how to do queries from C++ code.
c

c Switch used to select whether all values or selected ones are queried.

c 
	program cencalvmf77query
	implicit none

	character*128 filenameIn
	integer unitIn
	parameter(unitIn=10)

        character*64 filenameOut
	integer unitOut
	parameter(unitOut=11)

        character*64 filenameDB
	parameter(filenameDB='USGSBayAreaVM-08.3.0.etree')

        character*64 filenameDBExt
	parameter(filenameDBExt='USGSBayAreaVMExt-08.3.0.etree')

        character*64 filenameLog
	parameter(filenameLog='test.log')

        character*64 queryType

	real*8 queryRes

	real*8 squashLimit
	parameter(squashLimit=-2000.0)

	integer*4 cacheSize
	parameter(cacheSize=128)

	character*256 errorMsg

	integer numVals
	parameter(numVals=9)
	real*8 vals(numVals)

	real*8 lon, lat, elev

	integer ok

c       ****************************************************************
c	UNCOMMENT ONE OF THE FOLLOWING LINES.
c
c       Note: Only 1 set can be uncommented at a time.
c       ****************************************************************

c       Query points reside only in the normal database
c       (Sets 1-3)
	parameter(filenameIn='../data/sample-05.1.x.in')

c       Query points reside in either the normal or extended database
c       (Sets 4-6)
c	parameter(filenameIn='../data/sample_ext-05.1.x.in')

c       ****************************************************************
c	UNCOMMENT ONE OF THE FOLLOWING SETS OF 3 LINES.
c
c       Note: Only 1 set can be uncommented at a time.
c       ****************************************************************

c       SET1 = Parameters for sample MAXRES query
	parameter(filenameOut='test_maxres.out')
	parameter(queryType='maxres')
	parameter(queryRes=0)

c       SET2 = Query resolution for fixedres query (dx=200m)
c	parameter(filenameOut='test_fixedres.out')
c	parameter(queryType='fixedres')
c	parameter(queryRes=200.0)

c       SET3 = Query resolution for waveres query (T=0.2s)
c	parameter(filenameOut='test_waveres.out')
c	parameter(queryType='waveres')
c	parameter(queryRes=0.2)

c
c       SETS 4-6 use the extended database
c
c       SET4 = Parameters for sample MAXRES query
c	parameter(filenameOut='testext_maxres.out')
c	parameter(queryType='maxres')
c	parameter(queryRes=0)

c       SET5 = Query resolution for fixedres query (dx=200m)
c	parameter(filenameOut='testext_fixedres.out')
c	parameter(queryType='fixedres')
c	parameter(queryRes=200.0)

c       SET6 = Query resolution for waveres query (T=0.2s)
c	parameter(filenameOut='testext_waveres.out')
c	parameter(queryType='waveres')
c	parameter(queryRes=0.1999)

c       ****************************************************************
c       Size of query and errHandler MUST match sizeof(void*) in C
c       ****************************************************************
	integer*8 query
	integer*8 errHandler

	ok = 0

c       Create query
	call cencalvm_createquery_f(query)
	if (query.eq.0) goto 999

c       Get handle to error handler
	call cencalvm_errorhandler_f(query, errHandler)
	if (errHandler.eq.0) goto 999

c       Set log file in error handler
	call cencalvm_error_logfilename_f(errHandler, filenameLog, ok)
	if (ok.ne.0) goto 999

c       Set database filename
	call cencalvm_filename_f(query, filenameDB, ok)
	if (ok.ne.0) goto 998

	call cencalvm_cachesize_f(query, cacheSize, ok)
	if (ok.ne.0) goto 998

c       Set extended database filename
c       ****************************************************************
c       Uncomment this to use the extended database
c       ****************************************************************
c	call cencalvm_filenameext_f(query, filenameDBExt, ok)
c	if (ok.ne.0) goto 998
c	call cencalvm_cachesizeext_f(query, cacheSize, ok)
c	if (ok.ne.0) goto 998

c       Set squashing limit
c       ****************************************************************
c       Uncomment this to use squashing of topography
c       ****************************************************************
c	call cencalvm_squash_f(query, 1, squashLimit, ok)
c	if (ok.ne.0) goto 998

c       Open database for querying
	call cencalvm_open_f(query, ok)
	if(ok.ne.0) goto 998

c       Set query type and resolution
	if(queryType(1:6).eq.'maxres') then
	   call cencalvm_querytype_f(query, 0, ok)
	   if(ok.ne.0) goto 998
	else
	   if(queryRes.lt.0.0) then
	      write(6,*) 'Query resolution must be a positive value.'
	      goto 999
	   endif
	   call cencalvm_queryres_f(query, queryRes, ok)
	   if(ok.ne.0) goto 998
	   if(queryType(1:8).eq.'fixedres') then
	      call cencalvm_querytype_f(query, 1, ok)
	      if(ok.ne.0) goto 998
	   else
	      if(queryType(1:7).eq.'waveres') then
		 call cencalvm_querytype_f(query, 2, ok)
	      else
		 write(6,*) 'Could not parse query resolution string.'
		 goto 999
	      endif
	   endif
	endif

c       Open input file to read locations
	open(unitIn,file=filenameIn,status='old')

c       Open output file to accept data
	open(unitOut,file=filenameOut,status='new')

c       Read location from input file
 10	read(unitIn,*,end=30) lon,lat,elev

c       Query database
	call cencalvm_query_f(query,vals,numVals,lon,lat,elev,ok)

	if(ok.ne.0) then
	   call cencalvm_error_message_f(errHandler, errorMsg, ok)
	   write(6,*) errorMsg
c          If query generated an error, then bail out, otherwise reset status
	   if (ok.eq.2) goto 999
	   call cencalvm_error_resetstatus_f(errHandler, ok)
	   if (ok.ne.0) goto 998
	endif

c       Write values returned by query to output file
	write(unitOut,20) lon,lat,elev,vals(1),vals(2),vals(3),
     &    vals(4),vals(5),vals(6),int(vals(7)),int(vals(8)),vals(9)
 20	format(f9.4,f8.4,f9.1,f8.1,f8.1,f8.1,f9.1,f9.1,f9.1,i5,i5,f9.1)

	goto 10

c       Close database
 30	call cencalvm_close_f(query, ok)
	if(ok.ne.0) goto 998

c       Close input and output files
	close(unitIn)
	close(unitOut)

c       Destroy query handle
	call cencalvm_destroyquery_f(query, ok)
	if(ok.ne.0) goto 999

	goto 100
 666	write(6,*) 'Error reading input file.'
	goto 999
 777	write(6,*) 'Error opening input file.'
	goto 999
 888	write(6,*) 'Error opening output file.'
	goto 999
 998	call cencalvm_error_message_f(errHandler, errorMsg, ok)
	write(6,*) errorMsg
	goto 999
 999	write(6,*) 'ERROR'
 100	end


c End of file
