#include "common/platform.h"
#include "fw/src/mgos_app.h"
#include "common/cs_dbg.h"
#include "fw/src/mgos_app.h"
#include "fw/src/mgos_mongoose.h"
#include "fw/src/mgos_utils.h"
#include "fw/src/mgos_hal.h"
#include "fw/src/mgos_sys_config.h"
#include "fw/src/mgos_timers.h"
#include "mgos_mqtt.h"

#include "mgos_bme280.h"

static void pub(struct mg_connection *c,const char *topic, const char *fmt, ...)
{
  char msg[200];
  va_list ap;
  int n;
  va_start(ap, fmt);
  n = vsprintf(msg, fmt, ap);
  va_end(ap);
  mg_mqtt_publish(c, topic, mgos_mqtt_get_packet_id(),
                  MG_MQTT_QOS(0), msg, n);
  LOG(LL_INFO, ("%s -> %s", get_cfg()->mqtt.pub, msg));
}

static void on_timer(void *arg) {
  sensor_data data;
  mgos_bme280_get_sensor_data(&data);
  struct mg_connection *c = mgos_mqtt_get_global_conn();
  if (c != NULL)
  {
    pub(c, get_cfg()->mqtt.pub,"field1=%0.2f&field2=%0.3f&field3=%0.3f", data.temperature, data.pressure, data.humidity);    
  }
  LOG(LL_INFO, ("Temperature: %0.2f\tPressure: %0.3f kPa\tHumidity: %0.3f %%\n", data.temperature, data.pressure, data.humidity));
  (void) arg;
}

enum mgos_app_init_result mgos_app_init(void) {
  sensor_data data;
  mgos_bme280_get_sensor_data(&data);
  // Initial reading
  LOG(LL_INFO, ("Temperature: %0.2f\tPressure: %0.3f kPa\tHumidity: %0.3f %%\n", data.temperature, data.pressure, data.humidity));
  mgos_set_timer(30000 /* ms */,
                    true /* repeat */,
                    on_timer, NULL);
  return MGOS_APP_INIT_SUCCESS;
}
