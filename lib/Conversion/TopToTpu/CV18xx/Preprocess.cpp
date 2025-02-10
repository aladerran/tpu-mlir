//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// TPU-MLIR is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//
#include "tpu_mlir/Conversion/TopToTpu/LoweringCV18xx.h"

#define DEBUG_TYPE "lowering-Preprocess"
namespace tpu_mlir {
namespace cv18xx {

static double gen_low_table(double val) {
  uint16_t val_l = f32_to_bf16(static_cast<float>(val), true);
  double lut_low = ((int8_t *)(&val_l))[0];
  return lut_low;
}
static double gen_high_table(double val) {
  uint16_t val_h = f32_to_bf16(static_cast<float>(val), true);
  double lut_high = ((int8_t *)(&val_h))[1];
  return lut_high;
}

void PreprocessLowering::LoweringINT8(PatternRewriter &rewriter,
                                      top::PreprocessOp op,
                                      bool asymmetric) const {
  Value low_table = create_lookup_table(op.getInput(), op.getOutput(),
                                        asymmetric, activate_f(gen_low_table));
  Value high_table = create_lookup_table(
      op.getInput(), op.getOutput(), asymmetric, activate_f(gen_high_table));
  auto newType = getQuantBF16Type(op.getOutput());
  double threshold = 0;
  float white_level = 4095.;
  float black_level = 112.;
  std::vector<int64_t> channel_order;
  std::string pixel_format = op.getCustomizationFormat().str();
  if (pixel_format == "BGGR_RAW") {
    channel_order.push_back(3);
    channel_order.push_back(2);
    channel_order.push_back(0);
    channel_order.push_back(1);
  } else if (pixel_format == "GRBG_RAW") {
    channel_order.push_back(1);
    channel_order.push_back(0);
    channel_order.push_back(2);
    channel_order.push_back(3);
  } else if (pixel_format == "RGGB_RAW") {
    channel_order.push_back(0);
    channel_order.push_back(1);
    channel_order.push_back(3);
    channel_order.push_back(2);
  } else if (pixel_format == "GBRG_RAW") {
    channel_order.push_back(2);
    channel_order.push_back(3);
    channel_order.push_back(1);
    channel_order.push_back(0);
  } else {
    llvm_unreachable("raw format not support current type");
  }
  std::vector<NamedAttribute> attrs;
  attrs.emplace_back(rewriter.getNamedAttr(
      "white_level", rewriter.getF64FloatAttr(white_level)));
  attrs.emplace_back(rewriter.getNamedAttr(
      "black_level", rewriter.getF64FloatAttr(black_level)));
  attrs.emplace_back(
      rewriter.getNamedAttr("threshold", rewriter.getF64FloatAttr(threshold)));
  attrs.emplace_back(rewriter.getNamedAttr(
      "channel_order", rewriter.getI64ArrayAttr(channel_order)));
  rewriter.replaceOpWithNewOp<tpu::PackRawOp>(
      op, newType, ValueRange{op.getInput(), high_table, low_table}, attrs);
}

void PreprocessLowering::LoweringBF16(PatternRewriter &rewriter,
                                      top::PreprocessOp op) const {
  Value low_table = create_lookup_table(op.getInput(), op.getOutput(), 0,
                                        activate_f(gen_low_table));
  Value high_table = create_lookup_table(op.getInput(), op.getOutput(), 0,
                                         activate_f(gen_high_table));
  auto newType = getQuantBF16Type(op.getOutput());
  double threshold = 0;
  float white_level = 4095.;
  float black_level = 112.;
  std::vector<int64_t> channel_order;
  std::string pixel_format = op.getCustomizationFormat().str();
  if (pixel_format == "BGGR_RAW") {
    channel_order.push_back(3);
    channel_order.push_back(2);
    channel_order.push_back(0);
    channel_order.push_back(1);
  } else if (pixel_format == "GRBG_RAW") {
    channel_order.push_back(1);
    channel_order.push_back(0);
    channel_order.push_back(2);
    channel_order.push_back(3);
  } else if (pixel_format == "RGGB_RAW") {
    channel_order.push_back(0);
    channel_order.push_back(1);
    channel_order.push_back(3);
    channel_order.push_back(2);
  } else if (pixel_format == "GBRG_RAW") {
    channel_order.push_back(2);
    channel_order.push_back(3);
    channel_order.push_back(1);
    channel_order.push_back(0);
  } else {
    llvm_unreachable("raw format not support current type");
  }
  std::vector<NamedAttribute> attrs;
  attrs.emplace_back(rewriter.getNamedAttr(
      "white_level", rewriter.getF64FloatAttr(white_level)));
  attrs.emplace_back(rewriter.getNamedAttr(
      "black_level", rewriter.getF64FloatAttr(black_level)));
  attrs.emplace_back(
      rewriter.getNamedAttr("threshold", rewriter.getF64FloatAttr(threshold)));
  attrs.emplace_back(rewriter.getNamedAttr(
      "channel_order", rewriter.getI64ArrayAttr(channel_order)));
  rewriter.replaceOpWithNewOp<tpu::PackRawOp>(
      op, newType, ValueRange{op.getInput(), high_table, low_table}, attrs);
}

} // namespace cv18xx
} // namespace tpu_mlir
