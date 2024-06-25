#include <arpa/inet.h>
#include <pybind11/pybind11.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

int add(int i, int j) { return i + j; }
namespace py = pybind11;
#pragma pack(1)
struct log_para_t {
  char sbs;
  uint16_t mean_adc_val;
  int32_t dac_val_p;
  int32_t dac_val_n;
  uint32_t checkBits;
};
#pragma pack()

#define DIFVAL (3.0 / 4095.0)
#define DIFVAL32 (3.0 / (1000.0 * 1000.0 * 4095.0))

float convertADC(uint16_t adc) { return adc * DIFVAL; }
int32_t convertADC32bit(int32_t adc) { return adc ; }

py::tuple parsePkg(char *src) {
  // printf("src:%s\n",src);
  if (*(src + 0) == '#') {
    struct log_para_t *plog = (struct log_para_t *)(src);

    // printf("mean_adc_val:0x%x(%d),dac_val_p:0x%x,dac_val_n:0x%x\n",
    //        plog->mean_adc_val, plog->mean_adc_val, plog->dac_val_p,
    //        plog->dac_val_n);
    uint32_t checkBits_tmp = ((uint32_t) plog->mean_adc_val)+
                             ((uint32_t) plog->dac_val_p)+
                             ((uint32_t) plog->dac_val_n);
    
    if(plog->checkBits == checkBits_tmp){
      
      return py::make_tuple(convertADC(plog->mean_adc_val), convertADC32bit((plog->dac_val_p)),
                          convertADC32bit((plog->dac_val_n)));
    }else{
      printf("checkBits %08x != %08x\n",plog->checkBits,checkBits_tmp);
      return py::make_tuple(-100, -100, -100);
    }
    
  } else {
    return py::make_tuple(-100, -100, -100);
  }
}

PYBIND11_MODULE(netOcd, m) {
  m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: cmake_example

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

  m.def("add", &add, R"pbdoc(
        Add two numbers

        Some other explanation about the add function.
    )pbdoc");

  m.def("parsePkg", &parsePkg, R"pbdoc(
        pars netOcd com pkg data

        Some other explanation about the add function.
    )pbdoc");

  m.def(
      "subtract", [](int i, int j) { return i - j; }, R"pbdoc(
        Subtract two numbers

        Some other explanation about the subtract function.
    )pbdoc");

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
