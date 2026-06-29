#ifndef ADCM_DF_H
#define ADCM_DF_H

#include <cstdint>
#include <sys/types.h>
#include <fstream>
#include <vector>
#include <set>


#define STOR_ID_CMAP  0x504D    /* 'MP' */
#define STOR_ID_EVNT  0x5645    /* 'EV' */
#define STOR_ID_CNTR  0x5443    /* 'CT' */

struct stor_packet_hdr_t {
  u_int16_t id;     // data block ID
  u_int16_t size;   // block size, bytes
  friend std::ifstream &operator >> (std::ifstream &stream, stor_packet_hdr_t &d) {
      stream.read(reinterpret_cast<char *>(&d), sizeof(stor_packet_hdr_t));
      return stream;
  }
} __attribute__ ((packed));

struct stor_ev_hdr_t {
  u_int8_t np;          // number of pulses following
  u_int8_t reserved1;   // unused
  u_int16_t reserved2;  // unused
  u_int32_t ts;         // timestamp, 10 ns step
  friend std::ifstream &operator >> (std::ifstream &stream, stor_ev_hdr_t &d) {
      stream.read(reinterpret_cast<char *>(&d), sizeof(stor_ev_hdr_t));
      return stream;
  }
} __attribute__ ((packed));


struct stor_puls_t {
  u_int8_t ch;      // channel number
  u_int8_t flags;   // ?
  float a;          // amplitude in channels
  float t;          // time in ns
  float w;          // signal raise time
  friend std::ifstream &operator >> (std::ifstream &stream, stor_puls_t &d) {
      stream.read(reinterpret_cast<char *>(&d), sizeof(stor_puls_t));
      return stream;
  }
} __attribute__ ((packed));

struct adcm_cmap_t {
  u_int32_t n;                  // number of channels
  std::vector<u_int8_t> map;    // channels map
  friend std::ifstream &operator >> (std::ifstream &stream, adcm_cmap_t &d) {
      stream.read(reinterpret_cast<char *>(&d.n), sizeof(u_int32_t));
      d.map.resize(d.n, 0);
      for (size_t i{0}; i < d.n; ++i)
      {
          stream.read(reinterpret_cast<char *>(&d.map[i]), sizeof(u_int8_t));
      }
      return stream;
  }
};

struct adcm_counters_t {
  u_int32_t n;                      // number of channels
  double time;                      // time of count measurement in seconds
  std::vector<u_int32_t> rawhits;   // raw hit counters
  friend std::ifstream &operator >> (std::ifstream &stream, adcm_counters_t &d) {
      stream.read(reinterpret_cast<char *>(&d.n), sizeof(u_int32_t));
      stream.read(reinterpret_cast<char *>(&d.time), sizeof(double));
      d.rawhits.resize(d.n, 0);
      for (size_t i{0}; i < d.n; ++i)
      {
         stream.read(reinterpret_cast<char *>(&d.rawhits[i]), sizeof(u_int32_t));
      }
      return stream;
  }
};


struct dec_det_t
{    
    uint8_t index;  // detector index
    float amp;      // detector amplitude
    float rt;       // detector signal raise time
} __attribute__ ((packed));

struct dec_ev_m_t {
    float tdc;
    float amp;
} __attribute__ ((packed));

struct dec_ev_t
{    
    uint32_t ts;    // timestamp, 10 ns step
    float tdc;      // delta time = gamma_time - alpha_time
    dec_det_t a;    // alpha detector
    dec_det_t g;    // gamma detector
} __attribute__ ((packed));

struct dec_ev_1_t
{
    uint32_t ts;    // timestamp, 10 ns step
    float tdc;      // time
    dec_det_t d;    // alpha-gamma detector
} __attribute__ ((packed));

struct dec_cnt_t
{
    double time;        // time of count measurement in seconds
    std::vector<u_int32_t> rawhits;  // raw hit counters
};

struct dec_ch_t
{
    std::vector<u_int8_t> g;
    std::vector<u_int8_t> a;
    std::vector<u_int8_t> d;
};


#endif /* ADCM_DF_H */
