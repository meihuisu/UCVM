#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucvm_utils.h"
#include "ucvm_model_plugin.h"

#ifndef _UCVM_AM_STATIC
	#include <dlfcn.h>
#endif

#ifdef _UCVM_AM_STATIC
#ifdef _UCVM_ENABLE_CVMS5
	extern int cvms5_init;
	extern int cvms5_query;
	extern int cvms5_finalize;
	extern int cvms5_version;
#endif
#endif

// Variables

/** Have we initialized this model yet? */
int plugin_model_initialized = 0;

/** Used to store the model ID. */
int ucvm_plugin_model_id = UCVM_SOURCE_NONE;

/** Store the configuration data. */
ucvm_modelconf_t ucvm_plugin_model_conf;

/** Stores the points in in WGS84 format. */
basic_point_t *pointsBuffer;

/** The returned data from the plugin model. */
basic_properties_t *dataBuffer;

/**
 * Initializes the model within the UCVM framework. This is accomplished
 * by dynamically loading the library symbols.
 *
 * @param The model's id.
 * @param The configuration file parameters for the model.
 * @return UCVM_CODE_SUCCESS on success or ERROR on failure.
 */
int ucvm_plugin_model_init(int id, ucvm_modelconf_t *conf) {
	void *handle;
	char sopath[1024];

	// Have we initialized this model already?
	if (plugin_model_initialized) {
		//fprintf(stderr, "Model %s has already been initialized.\n", conf->label);
		//return UCVM_CODE_ERROR;
	}

#ifndef _UCVM_AM_STATIC
	snprintf(sopath, 1024, "%s/model/%s/lib/lib%s.so", conf->config, conf->label, conf->label);

	// Open the library.
	handle = dlopen (sopath, RTLD_LAZY);

	if (!handle) {
		fprintf(stderr, "Could not load %s. Error: %s.\n", conf->label, dlerror());
		return UCVM_CODE_ERROR;
	}

	// Load the symbols.
	model_init = dlsym(handle, "model_init");

	if (dlerror() != NULL) {
		fprintf(stderr, "Could not load model_init.\n");
		return UCVM_CODE_ERROR;
	}

	model_query = dlsym(handle, "model_query");

	if (dlerror() != NULL) {
		fprintf(stderr, "Could not load model_query.\n");
		return UCVM_CODE_ERROR;
	}

	model_finalize = dlsym(handle, "model_finalize");

	if (dlerror() != NULL) {
		fprintf(stderr, "Could not load model_finalize.\n");
		return UCVM_CODE_ERROR;
	}

	model_version = dlsym(handle, "model_version");

	if (dlerror() != NULL) {
		fprintf(stderr, "Could not load model_version.\n");
		return UCVM_CODE_ERROR;
	}

	// Initialize the model.
	if ((*model_init)(conf->config, conf->label) != 0) {
		fprintf(stderr, "Failed to initialize model, %s.\n", conf->label);
		return UCVM_CODE_ERROR;
	}
#endif

#ifdef _UCVM_AM_STATIC
#ifdef _UCVM_ENABLE_CVMS5
        if (strcmp(conf->label, "cvms5") == 0) {
                model_init = &cvms5_init;
                model_query = &cvms5_query;
                model_finalize = &cvms5_finalize;
                model_version = &cvms5_version;
                if ((*model_init)(conf->config, conf->label) != 0) {
                        fprintf(stderr, "Failed to initialize model, %s.\n", conf->label);
                        return UCVM_CODE_ERROR;
                }
        }
#endif

        if (model_init == NULL) {
                fprintf(stderr, "Model pointer not initialized.\n");
                return UCVM_CODE_ERROR;
        }

#endif

	// Assign the id.
	ucvm_plugin_model_id = id;

	// Save the model configuration data.
	memcpy(&ucvm_plugin_model_conf, conf, sizeof(ucvm_modelconf_t));

	// Allocate our point and data buffers.
	pointsBuffer = malloc(MODEL_POINT_BUFFER * sizeof(basic_point_t));
	dataBuffer = malloc(MODEL_POINT_BUFFER * sizeof(basic_properties_t));

	// Yes, this model has been initialized successfully.
	plugin_model_initialized = 1;

	return UCVM_CODE_SUCCESS;
}

/**
 * Finalizes the model and recovers all allocated memory.
 *
 * @return UCVM_CODE_SUCCESS
 */
int ucvm_plugin_model_finalize()
{
  if (plugin_model_initialized != 0) {
    // Finalize the model.
    (*model_finalize)();
  }

  // Free our pointers.
  free(dataBuffer);
  free(pointsBuffer);

  // We're no longer initialized.
  plugin_model_initialized = 0;

  return UCVM_CODE_SUCCESS;
}

/**
 * Retrieves the version of the model with which we're working.
 *
 * @param The model id.
 * @param The version string to be returned.
 * @param The maximum length of the version string.
 * @return UCVM_CODE_SUCCESS if everything works, ERROR if not.
 */
int ucvm_plugin_model_version(int id, char *ver, int len)
{
  if (id != ucvm_plugin_model_id) {
    fprintf(stderr, "Invalid model id.\n");
    return UCVM_CODE_ERROR;
  }

  if ((*model_version)(ver, len) != 0) {
    return UCVM_CODE_ERROR;
  }

  return UCVM_CODE_SUCCESS;
}

/**
 * Retrieves the model's label.
 *
 * @param The id of the model.
 * @param The label to return.
 * @param The maximum length of the label.
 */
int ucvm_plugin_model_label(int id, char *lab, int len)
{
  if (id != ucvm_plugin_model_id) {
    fprintf(stderr, "Invalid model id.\n");
    return UCVM_CODE_ERROR;
  }

  ucvm_strcpy(lab, ucvm_plugin_model_conf.label, len);
  return UCVM_CODE_SUCCESS;
}

/**
 * This is the main function to query the model. It transfers to the point
 * buffer and then queries the model.
 *
 * @param The model id.
 * @param Co-ordinate mode (elevation or depth or neither).
 * @param The number of points in the point buffer.
 * @param The point buffer.
 * @param The data buffer.
 */
int ucvm_plugin_model_query(int id, ucvm_ctype_t cmode, int n, ucvm_point_t *pnt, ucvm_data_t *data) {
	int i = 0, j = 0, nn = 0;
	int numTimesNN = 0;
	double depth = 0;
	int datagap = 0;

	// Check the model id.
	if (id != ucvm_plugin_model_id) {
		fprintf(stderr, "Invalid model id.\n");
		return UCVM_CODE_ERROR;
	}

	// Ensure we're using depth or elevation queries.
	if (cmode != UCVM_COORD_GEO_DEPTH && cmode != UCVM_COORD_GEO_ELEV) {
		fprintf(stderr, "Unsupported coordinate system type.\n");
		return UCVM_CODE_ERROR;
	}

	for (i = 0; i < n; i++) {
	    if ((data[i].crust.source == UCVM_SOURCE_NONE) && ((data[i].domain == UCVM_DOMAIN_INTERP) || (data[i].domain == UCVM_DOMAIN_CRUST)) &&
	      	(region_contains_null(&(ucvm_plugin_model_conf.region), cmode, &(pnt[i])))) {

	        /* Modify pre-computed depth to account for GTL interp range */
	        depth = data[i].depth + data[i].shift_cr;

    		pointsBuffer[nn].longitude = pnt[i].coord[0];
    		pointsBuffer[nn].latitude = pnt[i].coord[1];
    		pointsBuffer[nn].depth = depth;
    		nn++;

	    	if (nn == MODEL_POINT_BUFFER || i == n - 1) {
	    		// We've reached the maximum buffer. Do the query.
	    		(*model_query)(pointsBuffer, dataBuffer, nn);

	    		// Transfer our findings.
	    		for (j = 0; j < nn; j++) {
	    			if (dataBuffer[j].vp >= 0 && dataBuffer[j].vs >= 0 && dataBuffer[j].rho >= 0) {
	    				data[(numTimesNN * MODEL_POINT_BUFFER) + j].crust.source = ucvm_plugin_model_id;
	    				data[(numTimesNN * MODEL_POINT_BUFFER) + j].crust.vp = dataBuffer[j].vp;
	    				data[(numTimesNN * MODEL_POINT_BUFFER) + j].crust.vs = dataBuffer[j].vs;
	    				data[(numTimesNN * MODEL_POINT_BUFFER) + j].crust.rho = dataBuffer[j].rho;
	    			} else {
	    				datagap = 1;
	    			}
	    		}

	    		// Reset nn.
	    		nn = 0;
    			// Increment numTimesNN.
    			numTimesNN++;
    		}
	    } else {
	    	if (data[i].crust.source == UCVM_SOURCE_NONE) datagap = 1;
	    }
	}

	if (datagap == 1) {
		return UCVM_CODE_DATAGAP;
	}

	return UCVM_CODE_SUCCESS;
}

/**
 * Fill model structure with the plugin model, if it exists.
 *
 * @param dir The directory in which UCVM was installed.
 * @param label The model label to be set.
 * @param m The model structure to be filled.
 * @return SUCCESS if the model found, FAIL if not.
 */
int ucvm_plugin_get_model(const char *dir, const char *label, ucvm_model_t *m) {
	// Does this model exist?
	char sofile[1024];
	FILE *file;

	snprintf(sofile, 1024, "%s/model/%s/lib/lib%s.so", dir, label, label);

    if (file = fopen(sofile, "r")) {
        fclose(file);
        m->mtype = UCVM_MODEL_CRUSTAL;
		m->init = ucvm_plugin_model_init;
		m->finalize = ucvm_plugin_model_finalize;
		m->getversion = ucvm_plugin_model_version;
		m->getlabel = ucvm_plugin_model_label;
		m->setparam = ucvm_plugin_model_setparam;
		m->query = ucvm_plugin_model_query;
		return UCVM_CODE_SUCCESS;
	} else{
		return UCVM_CODE_ERROR;
	}
}

/* Setparam CVM-S5 */
int ucvm_plugin_model_setparam(int id, int param, ...)
{
  va_list ap;

  if (id != ucvm_plugin_model_id) {
    fprintf(stderr, "Invalid model id\n");
    return(UCVM_CODE_ERROR);
  }

  va_start(ap, param);
  switch (param) {
  default:
    break;
  }

  va_end(ap);

  return(UCVM_CODE_SUCCESS);
}
