# GNU Automake config

EXTRA_DIST = ucvm_template.conf

CHANGE_LIST=ucvm.conf

# USGS CenCalVM config
if UCVM_AM_ENABLE_CENCAL
CHANGE_LIST += cencal_model cencal_extmodel
endif

# CVM-H config
if UCVM_AM_ENABLE_CVMH
CHANGE_LIST += cvmh_model
endif

# CVM-S config
if UCVM_AM_ENABLE_CVMS
CHANGE_LIST += cvms_model
endif

# CVM-SI config
if UCVM_AM_ENABLE_CVMSI
CHANGE_LIST += cvmsi_model
endif

# CVM-NCI config
if UCVM_AM_ENABLE_CVMNCI
CHANGE_LIST += cvmnci_model
endif

# WFCVM config
if UCVM_AM_ENABLE_WFCVM
CHANGE_LIST += wfcvm_model
endif

# CVMLT config
if UCVM_AM_ENABLE_CVMLT
CHANGE_LIST += cvmlt_model
endif

# CMRG config
if UCVM_AM_ENABLE_CMRG
CHANGE_LIST += cmrg_model
endif

# Tape config
if UCVM_AM_ENABLE_TAPE
CHANGE_LIST += tape_model
endif

# SED Command
if USER_MAC
SED=sed -i ''
endif

if USER_LINUX
SED=sed -i''
endif

.PHONY=cencal_model cencal_extmodel cvmh_model \
	cvms_model cvmsi_model cvmnci_model \
	wfcvm_model cvmlt_model cmrg_model

all: ${CHANGE_LIST}

ucvm.conf: ucvm_template.conf
	cp $^ $@

cencal_model: ucvm.conf
	${SED} "s,#cencal_modelpath,cencal_modelpath,g" $^
	${SED} "s,CENCAL_MODEL,${CENCAL_MODEL},g" $^

cencal_extmodel: ucvm.conf
	${SED} "s,#cencal_extmodelpath,cencal_extmodelpath,g" $^
	${SED} "s,CENCAL_EXTMODEL,${CENCAL_EXTMODEL},g" $^

cvmh_model: ucvm.conf
	${SED} "s,#cvmh_modelpath,cvmh_modelpath,g" $^
	${SED} "s,CVMH_MODEL,${CVMH_MODEL},g" $^

cvms_model: ucvm.conf
	${SED} "s,#cvms_modelpath,cvms_modelpath,g" $^
	${SED} "s,CVMS_MODEL,${CVMS_MODEL},g" $^

cvmsi_model: ucvm.conf
	${SED} "s,#cvmsi_modelpath,cvmsi_modelpath,g" $^
	${SED} "s,CVMSI_MODEL,${CVMSI_MODEL},g" $^

cvmnci_model: ucvm.conf
	${SED} "s,#cvmnci_modelpath,cvmnci_modelpath,g" $^
	${SED} "s,CVMNCI_MODEL,${CVMNCI_MODEL},g" $^

wfcvm_model: ucvm.conf
	${SED} "s,#wfcvm_modelpath,wfcvm_modelpath,g" $^
	${SED} "s,WFCVM_MODEL,${WFCVM_MODEL},g" $^

cvmlt_model: ucvm.conf
	${SED} "s,#cvmlt_modelpath,cvmlt_modelpath,g" $^
	${SED} "s,CVMLT_MODEL,${CVMLT_MODEL},g" $^

cmrg_model: ucvm.conf
	${SED} "s,#cmrg_modelpath,cmrg_modelpath,g" $^
	${SED} "s,CMRG_MODEL,${CMRG_MODEL},g" $^

tape_model: ucvm.conf
	${SED} "s,#tape_modelpath,tape_modelpath,g" $^
	${SED} "s,TAPE_MODEL,${TAPE_MODEL},g" $^

install:

clean:
	rm -rf ucvm.conf
