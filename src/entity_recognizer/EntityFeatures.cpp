// Copyright (c) 2012-2015 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.3.
//
// TurboParser 2.3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.3.  If not, see <http://www.gnu.org/licenses/>.

#include "EntityPipe.h"
#include "EntityFeatures.h"
#include "EntityFeatureTemplates.h"
#include "SequencePart.h"
#include <bitset>

void EntityFeatures::AddUnigramFeatures(SequenceInstanceNumeric *sentence,
                                        int position) {

  CHECK(!input_features_unigrams_[position]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_unigrams_[position] = features;

  int sentence_length = sentence->size();

  EntityInstanceNumeric *entity_sentence =
    static_cast<EntityInstanceNumeric*>(sentence);

  EntityOptions *options = static_cast<class EntityPipe*>(pipe_)->
    GetEntityOptions();

  // Array of form IDs.
  const vector<int>* word_ids = &entity_sentence->GetFormIds();

  // Array of POS IDs.
  const vector<int>* pos_ids = &entity_sentence->GetPosIds();

  // Words.
  uint16_t WID = (*word_ids)[position]; // Current word.
  // Word on the left.
  uint16_t pWID = (position > 0) ? (*word_ids)[position - 1] : TOKEN_START;
  // Word on the right.
  uint16_t nWID = (position < sentence_length - 1) ?
    (*word_ids)[position + 1] : TOKEN_STOP;
  // Word two positions on the left.
  uint16_t ppWID = (position > 1) ? (*word_ids)[position - 2] : TOKEN_START;
  // Word two positions on the right.
  uint16_t nnWID = (position < sentence_length - 2) ?
    (*word_ids)[position + 2] : TOKEN_STOP;

  // Gazetteer tags.
  std::vector<int> empty_GIDs;
  // Current gazetter tag.
  const std::vector<int> &GIDs = entity_sentence->GetGazetteerIds(position);
  // Gazetteer tag on the left.
  const std::vector<int> &pGIDs = (position > 0) ?
    entity_sentence->GetGazetteerIds(position - 1) : empty_GIDs;
  // Gazetteer tag on the right.
  const std::vector<int> &nGIDs = (position < sentence_length - 1) ?
    entity_sentence->GetGazetteerIds(position + 1) : empty_GIDs;
  // Gazetteer tag two positions on the left.
  const std::vector<int> &ppGIDs = (position > 1) ?
    entity_sentence->GetGazetteerIds(position - 2) : empty_GIDs;
  // Gazetteer tag two positions on the right.
  const std::vector<int> &nnGIDs = (position < sentence_length - 2) ?
    entity_sentence->GetGazetteerIds(position + 2) : empty_GIDs;

  // POS tags.
  uint8_t PID = (*pos_ids)[position]; // Current POS.
  // POS on the left.
  uint8_t pPID = (position > 0) ?
    (*pos_ids)[position - 1] : TOKEN_START;
  // POS on the right.
  uint8_t nPID = (position < sentence_length - 1) ?
    (*pos_ids)[position + 1] : TOKEN_STOP;
  // POS two positions on the left.
  uint8_t ppPID = (position > 1) ?
    (*pos_ids)[position - 2] : TOKEN_START;
  // POS two positions on the right.
  uint8_t nnPID = (position < sentence_length - 2) ?
    (*pos_ids)[position + 2] : TOKEN_STOP;

  // Word shapes.
  uint16_t SID = sentence->GetShapeId(position); // Current shape.
  // Shape on the left.
  uint16_t pSID = (position > 0) ?
    sentence->GetShapeId(position - 1) : TOKEN_START;
  // Shape on the right.
  uint16_t nSID = (position < sentence_length - 1) ?
    sentence->GetShapeId(position + 1) : TOKEN_STOP;
  // Shape two positions on the left.
  uint16_t ppSID = (position > 1) ?
    sentence->GetShapeId(position - 2) : TOKEN_START;
  // Shape two positions on the right.
  uint16_t nnSID = (position < sentence_length - 2) ?
    sentence->GetShapeId(position + 2) : TOKEN_STOP;

  // Prefixes/Suffixes.
  vector<uint16_t> AID(sentence->GetMaxPrefixLength(position), 0xffff);
  vector<uint16_t> ZID(sentence->GetMaxSuffixLength(position), 0xffff);
  for (int l = 0; l < AID.size(); ++l) {
    AID[l] = sentence->GetPrefixId(position, l + 1);
  }
  for (int l = 0; l < ZID.size(); ++l) {
    ZID[l] = sentence->GetSuffixId(position, l + 1);
  }

  // Several flags.
  uint8_t flag_all_digits = sentence->AllDigits(position) ? 0x1 : 0x0;
  uint8_t flag_all_digits_with_punctuation =
    sentence->AllDigitsWithPunctuation(position) ? 0x1 : 0x0;
  uint8_t flag_all_upper = sentence->AllUpper(position) ? 0x1 : 0x0;
  uint8_t flag_first_upper = position > 0 && sentence->FirstUpper(position) ?
    0x1 : 0x0;

  flag_all_digits = 0x0 | (flag_all_digits << 4);
  flag_all_digits_with_punctuation =
    0x1 | (flag_all_digits_with_punctuation << 4);
  flag_all_upper = 0x2 | (flag_all_upper << 4);
  flag_first_upper = 0x3 | (flag_first_upper << 4);

  uint64_t fkey;
  vector<uint64_t> * multibit_fkey;
  uint8_t flags = 0x0;

  // Maximum is 255 feature templates.
  CHECK_LT(EntityFeatureTemplateUnigram::COUNT, 128);

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(EntityFeatureTemplateUnigram::BIAS,
                                  flags);
  AddFeature(fkey, features);

  // Lexical features.
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::W,
                               WID,
                               flags);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::pW,
                               pWID,
                               flags);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::nW,
                               nWID,
                               flags);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::ppW,
                               ppWID,
                               flags);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::nnW,
                               nnWID,
                               flags);
  AddFeature(fkey, features);

  // Gazetteer features.
  //G
  {
    std::vector<uint16_t> uint16_t_GIDs(GIDs.size());
    for (int k = 0; k < GIDs.size(); ++k) {
      uint16_t_GIDs[k] = GIDs[k];
    }
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateUnigram::G,
                                     &uint16_t_GIDs);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }
  //pG
  {
    std::vector<uint16_t> uint16_t_pGIDs(pGIDs.size());
    for (int k = 0; k < pGIDs.size(); ++k) {
      uint16_t_pGIDs[k] = pGIDs[k];
    }
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateUnigram::pG,
                                     &uint16_t_pGIDs);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  //nG
  {
    std::vector<uint16_t> uint16_t_nGIDs(nGIDs.size());
    for (int k = 0; k < nGIDs.size(); ++k) {
      uint16_t_nGIDs[k] = nGIDs[k];
    }
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateUnigram::nG,
                                     &uint16_t_nGIDs);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  //ppG
  {
    std::vector<uint16_t> uint16_t_ppGIDs(ppGIDs.size());
    for (int k = 0; k < ppGIDs.size(); ++k) {
      uint16_t_ppGIDs[k] = ppGIDs[k];
    }
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateUnigram::ppG,
                                     &uint16_t_ppGIDs);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  //nnG
  {
    std::vector<uint16_t> uint16_t_nnGIDs(nnGIDs.size());
    for (int k = 0; k < nnGIDs.size(); ++k) {
      uint16_t_nnGIDs[k] = nnGIDs[k];
    }
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateUnigram::nnG,
                                     &uint16_t_nnGIDs);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  // POS features.
  //P
  {
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_P(EntityFeatureTemplateUnigram::P,
                                     PID);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  //:PpP
  {
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_PP(EntityFeatureTemplateUnigram::PpP,
                                      PID, pPID);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  //PnP
  {
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_PP(EntityFeatureTemplateUnigram::PnP,
                                      PID, nPID);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  //PpPppP
  {
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_PPP(EntityFeatureTemplateUnigram::PpPppP,
                                       PID, pPID, ppPID);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  //PnPnnP
  {
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_PPP(EntityFeatureTemplateUnigram::PnPnnP,
                                       PID, nPID, nnPID);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  // Shape features.
  //S
  {
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateUnigram::S,
                                     SID);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  //pS
  {
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateUnigram::pS,
                                     pSID);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  //nS
  {
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateUnigram::nS,
                                     nSID);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  //ppS
  {
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateUnigram::ppS,
                                     ppSID);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  //nnS
  {
    multibit_fkey =
      encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateUnigram::nnS,
                                     nnSID);
    for (auto const &key : (*multibit_fkey))
      AddFeature(key, features);
    delete multibit_fkey;
  }

  // Prefix/Suffix features.
  for (int l = 0; l < AID.size(); ++l) {
    uint8_t flag_prefix_length = l;
    fkey = encoder_.CreateFKey_WP(EntityFeatureTemplateUnigram::A,
                                  AID[l], flag_prefix_length,
                                  flags);
    AddFeature(fkey, features);
  }
  for (int l = 0; l < ZID.size(); ++l) {
    uint8_t flag_suffix_length = l;
    fkey = encoder_.CreateFKey_WP(EntityFeatureTemplateUnigram::Z,
                                  ZID[l], flag_suffix_length,
                                  flags);
    AddFeature(fkey, features);
  }

  // Several flags.
  fkey = encoder_.CreateFKey_P(EntityFeatureTemplateUnigram::FLAG,
                               flag_all_digits,
                               flags);
  AddFeature(fkey, features);;

  fkey = encoder_.CreateFKey_P(EntityFeatureTemplateUnigram::FLAG,
                               flag_all_digits_with_punctuation,
                               flags);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_P(EntityFeatureTemplateUnigram::FLAG,
                               flag_all_upper,
                               flags);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_P(EntityFeatureTemplateUnigram::FLAG,
                               flag_first_upper,
                               flags);
  AddFeature(fkey, features);
}

void EntityFeatures::AddBigramFeatures(SequenceInstanceNumeric *sentence,
                                       int position) {

  CHECK(!input_features_bigrams_[position]) << position
    << " " << sentence->size();
  BinaryFeatures *features = new BinaryFeatures;
  input_features_bigrams_[position] = features;

  uint64_t fkey;
  vector<uint64_t> * multibit_fkey;
  uint8_t flags = 0x0;
  flags |= EntityFeatureTemplateParts::BIGRAM;

  // Note: position ranges between 0 and N (inclusive), where N is the number
  // of words. If position = N, we need to be careful not to access invalid
  // memory in arrays.

  // Add other bigram features.
  int sentence_length = sentence->size();

  EntityInstanceNumeric *entity_sentence =
    static_cast<EntityInstanceNumeric*>(sentence);

  EntityOptions *options = static_cast<class EntityPipe*>(pipe_)->
    GetEntityOptions();
  std::bitset<32> *feature_set_bitmap;
  feature_set_bitmap = new bitset<32>(options->large_feature_set());

  // Array of form IDs.
  const vector<int>* word_ids = &entity_sentence->GetFormIds();

  // Array of POS IDs.
  const vector<int>* pos_ids = &entity_sentence->GetPosIds();

  // Words.
  uint16_t WID, pWID, nWID, ppWID, nnWID;
  if (feature_set_bitmap->test(0)) {
    WID = (position < sentence_length) ?
      (*word_ids)[position] : TOKEN_STOP; // Current word.
    // Word on the left.
    pWID = (position > 0) ?
      (*word_ids)[position - 1] : TOKEN_START;
    // Word on the right.
    nWID = (position < sentence_length - 1) ?
      (*word_ids)[position + 1] : TOKEN_STOP;
    // Word two positions on the left.
    ppWID = (position > 1) ?
      (*word_ids)[position - 2] : TOKEN_START;
    // Word two positions on the right.
    nnWID = (position < sentence_length - 2) ?
      (*word_ids)[position + 2] : TOKEN_STOP;
  }

  // POS tags.
  uint8_t PID, pPID, nPID, ppPID, nnPID;
  if (feature_set_bitmap->test(1)) {
    PID = (position < sentence_length) ?
      (*pos_ids)[position] : TOKEN_STOP; // Current POS.
    // POS on the left.
    pPID = (position > 0) ?
      (*pos_ids)[position - 1] : TOKEN_START;
    // POS on the right.
    nPID = (position < sentence_length - 1) ?
      (*pos_ids)[position + 1] : TOKEN_STOP;
    // POS two positions on the left.
    ppPID = (position > 1) ?
      (*pos_ids)[position - 2] : TOKEN_START;
    // POS two positions on the right.
    nnPID = (position < sentence_length - 2) ?
      (*pos_ids)[position + 2] : TOKEN_STOP;
  }

  // Word shapes.
  uint16_t SID, pSID, nSID, ppSID, nnSID;
  if (feature_set_bitmap->test(2)) {
    SID = (position < sentence_length) ?
      sentence->GetShapeId(position) : TOKEN_STOP; // Current shape.
     // Shape on the left.
    pSID = (position > 0) ?
      sentence->GetShapeId(position - 1) : TOKEN_START;
    // Shape on the right.
    nSID = (position < sentence_length - 1) ?
      sentence->GetShapeId(position + 1) : TOKEN_STOP;
    // Shape two positions on the left.
    ppSID = (position > 1) ?
      sentence->GetShapeId(position - 2) : TOKEN_START;
    // Shape two positions on the right.
    nnSID = (position < sentence_length - 2) ?
      sentence->GetShapeId(position + 2) : TOKEN_STOP;
  }

  // Maximum is 255 feature templates.
  CHECK_LT(EntityFeatureTemplateBigram::COUNT, 128);

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(EntityFeatureTemplateBigram::BIAS,
                                  flags);
  AddFeature(fkey, features);

  // Lexical features.
  if (feature_set_bitmap->test(0)) {
    //Word
    fkey = encoder_.CreateFKey_W(EntityFeatureTemplateBigram::W,
                                 WID,
                                 flags);
    AddFeature(fkey, features);
    //pContext
    fkey = encoder_.CreateFKey_W(EntityFeatureTemplateBigram::pW,
                                 pWID,
                                 flags);
    AddFeature(fkey, features);
    //nContext
    fkey = encoder_.CreateFKey_W(EntityFeatureTemplateBigram::nW,
                                 nWID,
                                 flags);
    AddFeature(fkey, features);
    //ppContext
    fkey = encoder_.CreateFKey_W(EntityFeatureTemplateBigram::ppW,
                                 ppWID,
                                 flags);
    AddFeature(fkey, features);
    //nnContext
    fkey = encoder_.CreateFKey_W(EntityFeatureTemplateBigram::nnW,
                                 nnWID,
                                 flags);
    AddFeature(fkey, features);
  }

  // POS features.
  if (feature_set_bitmap->test(1)) {
    //P
    {
      multibit_fkey = encoder_.CreateFKey_MultiBit_P(EntityFeatureTemplateBigram::P,
                                                     PID);
      for (auto const &key : (*multibit_fkey))
        AddFeature(key,
                   features);
      delete multibit_fkey;
    }
    //PpP
    {
      multibit_fkey = encoder_.CreateFKey_MultiBit_PP(EntityFeatureTemplateBigram::PpP,
                                                      PID, pPID);
      for (auto const &key : (*multibit_fkey))
        AddFeature(key,
                   features);
      delete multibit_fkey;
    }
    //PnP
    {
      multibit_fkey = encoder_.CreateFKey_MultiBit_PP(EntityFeatureTemplateBigram::PnP,
                                                      PID, nPID);
      for (auto const &key : (*multibit_fkey))
        AddFeature(key,
                   features);
      delete multibit_fkey;
    }
    //PpPppP
    {
      multibit_fkey = encoder_.CreateFKey_MultiBit_PPP(EntityFeatureTemplateBigram::PpPppP,
                                                       PID, pPID, ppPID);
      for (auto const &key : (*multibit_fkey))
        AddFeature(key,
                   features);
      delete multibit_fkey;
    }
    //PnPnnP
    {
      multibit_fkey = encoder_.CreateFKey_MultiBit_PPP(EntityFeatureTemplateBigram::PnPnnP,
                                                       PID, nPID, nnPID);
      for (auto const &key : (*multibit_fkey))
        AddFeature(key,
                   features);
      delete multibit_fkey;
    }
  }

  // Shape features.
  if (feature_set_bitmap->test(2)) {
    //S
    {
      multibit_fkey = encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateBigram::S,
                                                     SID);
      for (auto const &key : (*multibit_fkey))
        AddFeature(key,
                   features);
      delete multibit_fkey;
    }

    //pS
    {
      multibit_fkey = encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateBigram::pS,
                                                     pSID);
      for (auto const &key : (*multibit_fkey))
        AddFeature(key,
                   features);
      delete multibit_fkey;
    }
    //nS
    {
      multibit_fkey = encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateBigram::nS,
                                                     nSID);
      for (auto const &key : (*multibit_fkey))
        AddFeature(key,
                   features);
      delete multibit_fkey;
    }
    //ppS
    {
      multibit_fkey = encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateBigram::ppS,
                                                     ppSID);
      for (auto const &key : (*multibit_fkey))
        AddFeature(key,
                   features);
      delete multibit_fkey;
    }
    //nnS
    {
      multibit_fkey = encoder_.CreateFKey_MultiBit_W(EntityFeatureTemplateBigram::nnS,
                                                     nnSID);
      for (auto const &key : (*multibit_fkey))
        AddFeature(key,
                   features);
      delete multibit_fkey;
    }
  }
  delete feature_set_bitmap;
}

void EntityFeatures::AddTrigramFeatures(SequenceInstanceNumeric *sentence,
                                        int position) {

  CHECK(!input_features_trigrams_[position]) << position
    << " " << sentence->size();
  BinaryFeatures *features = new BinaryFeatures;
  input_features_trigrams_[position] =features;

  uint64_t fkey;
  uint8_t flags = 0x0;
  flags |= EntityFeatureTemplateParts::TRIGRAM;

  // Maximum is 255 feature templates.
  CHECK_LT(EntityFeatureTemplateTrigram::COUNT, 128);

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(EntityFeatureTemplateTrigram::BIAS,
                                  flags);
  AddFeature(fkey, features);
}
