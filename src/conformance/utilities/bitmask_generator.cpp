// Copyright (c) 2019-2023, The Khronos Group Inc.
// Copyright (c) 2019 Collabora, Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/*!
 * @file
 * @brief  Implementation
 * @author Ryan Pavlik <ryan.pavlik@collabora.com>
 */

#include "bitmask_generator.h"
#include "generator.h"

#include <stddef.h>
#include <vector>
#include <memory>
#include <cstdint>

namespace Conformance
{
    namespace
    {

        /*!
         * GeneratorBase implementation for the BitmaskGenerator - implementation details.
         *
         * @see bitmaskGenerator for the factory function to create one of these.
         *
         * Uses the binary of an integer index as a selection of
         * which supplied bitmasks should be enabled in a given generated output.
         * Yes, this is a bitmask that selects bitmasks.
         */
        class BitmaskGenerator : public GeneratorBase<uint64_t const&>
        {
        public:
            ~BitmaskGenerator() override = default;

            BitmaskGenerator(const BitmaskGenerator&) = delete;
            BitmaskGenerator& operator=(const BitmaskGenerator&) = delete;
            BitmaskGenerator(BitmaskGenerator&&) = delete;
            BitmaskGenerator& operator=(BitmaskGenerator&&) = delete;

            static std::unique_ptr<GeneratorBase<uint64_t const&>> create(bool zeroOk, std::initializer_list<uint64_t> const& bits)
            {
                std::unique_ptr<BitmaskGenerator> generator(new BitmaskGenerator(zeroOk, bits));
                return generator;
            }

            BitmaskGenerator(bool zeroOk, std::initializer_list<uint64_t> const& bits) : bits_(bits), zeroOk_(zeroOk)
            {
            }

            uint64_t const& get() override
            {
                return current_;
            }

            bool next() override
            {
                // Return the zeroth combination first.
                if (zeroOk_ && !gotZeroYet_) {
                    gotZeroYet_ = true;
                    return true;
                }
                // Otherwise, move on to the next index
                currentIndex_++;
                const auto n = bits_.size();
                if (currentIndex_ >= (uint64_t(0x1) << n)) {
                    // n is highest bit number + 1.
                    // Thus, the largest mask is 0x1 << n - 1.
                    // If we exceed that, we've run out of combinations.
                    return false;
                }

                uint64_t accumulate = 0;
                // Loop through the bits of our index to determine whether to enable a given bitmask
                for (size_t i = 0; i < n; ++i) {
                    uint64_t indexBit = (uint64_t(0x1) << i);
                    if ((indexBit & currentIndex_) != 0) {
                        // Yes, enable this bitmask
                        accumulate |= bits_[i];
                    }
                }
                current_ = accumulate;
                return true;
            }

        private:
            std::vector<uint64_t> bits_;
            bool zeroOk_;
            bool gotZeroYet_ = false;
            uint64_t currentIndex_ = 0;
            uint64_t current_ = 0;
        };

    }  // namespace

    GeneratorWrapper<uint64_t const&> bitmaskGeneratorIncluding0(std::initializer_list<uint64_t> const& bits)
    {
        return GeneratorWrapper<uint64_t const&>(BitmaskGenerator::create(true, bits));
    }

    GeneratorWrapper<uint64_t const&> bitmaskGenerator(std::initializer_list<uint64_t> const& bits)
    {
        return GeneratorWrapper<uint64_t const&>(BitmaskGenerator::create(false, bits));
    }
}  // namespace Conformance
