/**
 * @file compute_control_modes.c
 * @brief Compute AO control modes in both input (WFS) and output (DM) space
 *
 */

#include "CommandLineInterface/CLIcore.h"

// Local variables pointers

static uint32_t *AOloopindex;

static float *svdlim;
static float *CPAmax;
static float *deltaCPA;

static float *alignCX; // X center
static float *alignCY; // Y center
static float *alignID; // Inner diameter
static float *alignOD; // Outer diameter

static uint32_t *DMxsize;
static uint32_t *DMysize;

static long fpi_FPS_zRMacqu;
static long fpi_FPS_loRMacqu;
static long fpi_FPS_DMcomb;

static char *fname_DMslaved;
static char *fname_zrespM;
static char *fname_DMmaskRM;
static char *fname_WFSmask;
static char *fname_loRM;
static char *fname_loRMmodes;




static CLICMDARGDEF farg[] = {{CLIARG_INT32,
                               ".AOloopindex",
                               "AO loop index",
                               "0",
                               CLIARG_VISIBLE_DEFAULT,
                               (void **) &AOloopindex,
                               NULL},
                              {CLIARG_FLOAT32,
                               ".svdlim",
                               "SVD limit",
                               "0.01",
                               CLIARG_VISIBLE_DEFAULT,
                               (void **) &svdlim,
                               NULL},
                              {CLIARG_FLOAT32,
                               ".CPAmax",
                               "max cycles per aperture (CPA)",
                               "20.0",
                               CLIARG_HIDDEN_DEFAULT,
                               (void **) &CPAmax,
                               NULL},
                              {CLIARG_FLOAT32,
                               ".deltaCPA",
                               "CPA increment",
                               "0.8",
                               CLIARG_HIDDEN_DEFAULT,
                               (void **) &deltaCPA,
                               NULL},
                              {CLIARG_FLOAT32,
                               ".align.CX",
                               "beam X center on DM",
                               "10.0",
                               CLIARG_HIDDEN_DEFAULT,
                               (void **) &alignCX,
                               NULL},
                              {CLIARG_FLOAT32,
                               ".align.CY",
                               "beam Y center on DM",
                               "10.0",
                               CLIARG_HIDDEN_DEFAULT,
                               (void **) &alignCY,
                               NULL},
                              {CLIARG_FLOAT32,
                               ".align.ID",
                               "beam inner diameter",
                               "5.0",
                               CLIARG_HIDDEN_DEFAULT,
                               (void **) &alignID,
                               NULL},
                              {CLIARG_FLOAT32,
                               ".align.OD",
                               "beam outer diameter",
                               "10.0",
                               CLIARG_HIDDEN_DEFAULT,
                               (void **) &alignOD,
                               NULL},
                              {CLIARG_UINT32,
                               ".DMxsize",
                               "DM x size",
                               "32",
                               CLIARG_HIDDEN_DEFAULT,
                               (void **) &DMxsize,
                               NULL},
                              {CLIARG_UINT32,
                               ".DMysize",
                               "DM y size",
                               "32",
                               CLIARG_HIDDEN_DEFAULT,
                               (void **) &DMysize,
                               NULL},
                              {CLIARG_STR,
                               ".FPS_zRMacqu",
                               "FPS zonal RM acquisition",
                               "NULL",
                               CLICMDARG_FLAG_NOCLI,
                               FPTYPE_FPSNAME,
                               FPFLAG_DEFAULT_INPUT | FPFLAG_FPS_RUN_REQUIRED,
                               (void **) &fpi_FPS_zRMacqu,
                               NULL},
                              {CLIARG_STR,
                               ".FPS_loRMacqu",
                               "FPS low order modal RM acquisition",
                               "NULL",
                               CLICMDARG_FLAG_NOCLI,
                               FPTYPE_FPSNAME,
                               FPFLAG_DEFAULT_INPUT | FPFLAG_FPS_RUN_REQUIRED,
                               (void **) &fpi_FPS_loRMacqu,
                               NULL},
                              {CLIARG_STR,
                               ".FPS_DMcomb",
                               "FPS DM comb",
                               "NULL",
                               CLICMDARG_FLAG_NOCLI,
                               FPTYPE_FPSNAME,
                               FPFLAG_DEFAULT_INPUT | FPFLAG_FPS_RUN_REQUIRED,
                               (void **) &fpi_FPS_DMcomb,
                               NULL},
                              {CLIARG_STR,
                               ".DMslaved",
                               "DM slaved actuators",
                               "NULL",
                               CLICMDARG_FLAG_NOCLI,
                               FPTYPE_FITSFILENAME,
                               FPFLAG_DEFAULT_INPUT | FPFLAG_FPS_RUN_REQUIRED,
                               (void **) &fname_DMslaved,
                               NULL},
                              {CLIARG_STR,
                               ".zrespM",
                               "zonal response matrix",
                               "NULL",
                               CLICMDARG_FLAG_NOCLI,
                               FPTYPE_FITSFILENAME,
                               FPFLAG_DEFAULT_INPUT | FPFLAG_FPS_RUN_REQUIRED,
                               (void **) &fname_zrespM,
                               NULL},
                              {CLIARG_STR,
                               ".DMmaskRM",
                               "actuators directly controlled",
                               "NULL",
                               CLICMDARG_FLAG_NOCLI,
                               FPTYPE_FITSFILENAME,
                               FPFLAG_DEFAULT_INPUT | FPFLAG_FPS_RUN_REQUIRED,
                               (void **) &fname_DMmaskRM,
                               NULL},
                              {CLIARG_STR,
                               ".WFSmask",
                               "WFS mask",
                               "NULL",
                               CLICMDARG_FLAG_NOCLI,
                               FPTYPE_FITSFILENAME,
                               FPFLAG_DEFAULT_INPUT | FPFLAG_FPS_RUN_REQUIRED,
                               (void **) &fname_WFSmask,
                               NULL},
                              {CLIARG_STR,
                               ".loRM",
                               "low order modal response matrix",
                               "NULL",
                               CLICMDARG_FLAG_NOCLI,
                               FPTYPE_FITSFILENAME,
                               FPFLAG_DEFAULT_INPUT | FPFLAG_FPS_RUN_REQUIRED,
                               (void **) &fname_loRM,
                               NULL},
                              {CLIARG_STR,
                               ".loRMmodes",
                               "low order RM modes",
                               "NULL",
                               CLICMDARG_FLAG_NOCLI,
                               FPTYPE_FITSFILENAME,
                               FPFLAG_DEFAULT_INPUT | FPFLAG_FPS_RUN_REQUIRED,
                               (void **) &fname_loRMmodes,
                               NULL}

};




static errno_t customCONFsetup()
{
    return RETURN_SUCCESS;
}




static errno_t customCONFcheck()
{
    if (data.fpsptr != NULL)
    {
        /* if (fps.parray[fpi_FPS_DMcomb].info.fps.FPSNBparamMAX < 1)
        {
            functionparameter_ConnectExternalFPS(&fps,
                                                 fpi_FPS_DMcomb,
                                                 &FPS_DMcomb);
        }*/
    }

    return RETURN_SUCCESS;
}




static CLICMDDATA CLIcmddata = {"compctrlmodes",
                                "compute AO control modes in WFS and DM space",
                                CLICMD_FIELDS_DEFAULTS};


// detailed help
static errno_t help_function()
{
    return RETURN_SUCCESS;
}




static errno_t compute_function()
{
    DEBUG_TRACE_FSTART();

    /*IMGID inimg = makeIMGID(inimname);
    resolveIMGID(&inimg, ERRMODE_ABORT);

    IMGID outimg = makeIMGID(outimname);
    resolveIMGID(&outimg, ERRMODE_ABORT);
    */
    INSERT_STD_PROCINFO_COMPUTEFUNC_INIT

    // custom initialization
    if (CLIcmddata.cmdsettings->flags & CLICMDFLAG_PROCINFO)
    {
        // procinfo is accessible here
    }

    INSERT_STD_PROCINFO_COMPUTEFUNC_LOOPSTART

    // streamprocess(inimg, outimg);
    // processinfo_update_output_stream(processinfo, outimg.ID);

    INSERT_STD_PROCINFO_COMPUTEFUNC_END

    DEBUG_TRACE_FEXIT();
    return RETURN_SUCCESS;
}

INSERT_STD_FPSCLIfunctions

    // Register function in CLI
    errno_t
    CLIADDCMD_cacao_computeCalib__compute_control_modes()
{
    CLIcmddata.FPS_customCONFsetup = customCONFsetup;
    CLIcmddata.FPS_customCONFcheck = customCONFcheck;
    INSERT_STD_CLIREGISTERFUNC

    return RETURN_SUCCESS;
}
