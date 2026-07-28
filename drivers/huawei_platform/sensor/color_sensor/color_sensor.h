/*
*****************************************************************************
* Copyright by HW                                                       *
* All rights are reserved.                                                  *
*                                                                           *
* IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
* THE SOFTWARE.                                                             *
*                                                                           *
* THIS SOFTWARE IS PROVIDED FOR USE ONLY IN CONJUNCTION WITH ROHM PRODUCTS.  *
* USE OF THE SOFTWARE IN CONJUNCTION WITH NON-ROHM-PRODUCTS IS EXPLICITLY    *
* EXCLUDED.                                                                 *
*                                                                           *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
*****************************************************************************
*/

/*
 * HW platform header
 */

/*
 * @@HW:
 */

#ifndef __COLOR_SENSOR_H__
#define	__COLOR_SENSOR_H__

#define UINT8   uint8_t
#define  INT8    int8_t
#define UINT16  uint16_t
#define  INT16   int16_t
#define UINT32  uint32_t
#define  INT32   int32_t

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum color_sensor_cal_states{
	CAL_STATE_GAIN_1,
	CAL_STATE_GAIN_2,
	CAL_STATE_GAIN_3,
	CAL_STATE_GAIN_4,
	CAL_STATE_GAIN_5,
	CAL_STATE_GAIN_LAST
}color_sensor_cal_states_t;

typedef struct color_sensor_cali_nv{
	uint32_t 	nv_Xtarget;
	uint32_t 	nv_Ytarget;
	uint32_t 	nv_Ztarget;
	uint32_t 	nv_IRtarget;
	uint32_t 	calXratio[CAL_STATE_GAIN_LAST];
	uint32_t 	calYratio[CAL_STATE_GAIN_LAST];
	uint32_t 	calZratio[CAL_STATE_GAIN_LAST];
	uint32_t 	calIratio[CAL_STATE_GAIN_LAST];
}color_sensor_cali_para_nv;

typedef struct color_sensor_input {
	UINT32 tar_x;
	UINT32 tar_y;
	UINT32 tar_z;
	UINT32 tar_ir;
	UINT32 enable;
}color_sensor_input_para;

typedef struct color_sensor_output {
	UINT32 result;
	UINT32 report_x[CAL_STATE_GAIN_LAST];
	UINT32 report_y[CAL_STATE_GAIN_LAST];
	UINT32 report_z[CAL_STATE_GAIN_LAST];
	UINT32 report_ir[CAL_STATE_GAIN_LAST];
}color_sensor_output_para;

typedef struct color_sensor_calibration{
	enum color_sensor_cal_states calState;
	uint32_t 	calXsample;
	uint32_t 	calYsample;
	uint32_t 	calZsample;
	uint32_t 	calIRsample;
	uint32_t 	calXtarget;
	uint32_t 	calYtarget;
	uint32_t 	calZtarget;
	uint32_t 	calIRtarget;
	uint32_t 	calXresult[CAL_STATE_GAIN_LAST];
	uint32_t 	calYresult[CAL_STATE_GAIN_LAST];
	uint32_t 	calZresult[CAL_STATE_GAIN_LAST];
	uint32_t 	calIRresult[CAL_STATE_GAIN_LAST];
	uint32_t  calSampleCounter;
}color_sensor_calibration_t;

struct colorDriver_chip {
	struct mutex lock;
	struct i2c_client *client;
	struct driver_i2c_platform_data *pdata;
	int in_suspend;
	int wake_irq;
	int irq_pending;
	bool unpowered;
	bool inCalMode;
	struct color_sensor_calibration calibrationCtx;
	u8 device_index;
	void * deviceCtx;
    struct timer_list work_timer;
    struct work_struct als_work;
	struct device *dev;
	void (*color_show_calibrate_state)(struct rohmDriver_chip *, color_sensor_output_para*);
	void (*color_store_calibrate_state)(struct rohmDriver_chip *, color_sensor_input_para*);
	void (*color_enable_show_state)(struct rohmDriver_chip *, int *);
	void (*color_enable_store_state)(struct rohmDriver_chip *, int);
	INT32 (*color_sensor_getGain)(void*);
	INT32 (*color_sensor_setGain)(void*, int);
};
#ifdef	__cplusplus
}
#endif

#endif /* __COLOR_SENSOR_H__ */
