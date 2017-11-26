#include "Rendering\peMaterial.h"

void pe::peMaterial::SetRealTimeData(const peRealTimeMaterialData &data) {
  auto storage = GetMutableDataStorage();
  storage->data = data;
}

void pe::peMaterial::SetOfflineData(const BSDF &data) {
  auto storage = GetMutableDataStorage();
  storage->data = data;
}

pe::peRealTimeMaterialData const *pe::peMaterial::GetRealTimeData() const {
  auto storage = GetDataStorage();
  return std::get_if<peRealTimeMaterialData>(&storage.data);
}

pe::peOfflineMaterialData const *pe::peMaterial::GetOfflineData() const {
  auto &storage = GetDataStorage();
  return std::get_if<peOfflineMaterialData>(&storage.data);
}
