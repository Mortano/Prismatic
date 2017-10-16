#pragma once

namespace pe
{

	namespace MemoryUtil
	{
		//Construct
		template<bool, typename T>
		struct UninitializedConstructImpl
		{
			static void uninitializedConstruct(T* dst, size_t numElements)
			{
				for(size_t i = 0; i < numElements; i++)
				{
					new (dst + i) T();
				}
			}
		};

		template<typename T>
		struct UninitializedConstructImpl<true, T>
		{
			static void uninitializedConstruct(T* dst, size_t numElements)
			{
				//Nothing to do
			}
		};

		template<typename T>
		void UninitializedConstruct(T* dst, size_t numElements)
		{
			//Taken from GEP. Nice compiler magic to call the right function depending on the type of T
			UninitializedConstructImpl<std::is_fundamental<T>::value || std::is_compound<T>::value && 
									   std::is_trivially_constructible<T>::value, T>
						::uninitializedConstruct(dst, numElements);
		}

		//Copy        
        template <bool, class T>
        struct uninitializedCopyImpl
        {
            static void uninitializedCopy(T* dst, const T* src, size_t count)
            {
                for(size_t i=0; i<count; i++)
                    new (dst+i) T(src[i]);
            }
        };

        // pod case
        template <class T>
        struct uninitializedCopyImpl<true, T>
        {
            static void uninitializedCopy(T* dst, const T* src, size_t count)
            {
                if(dst + count < src || dst > src + count) 
                    memcpy(dst, src, sizeof(T) * count);
                else
                    memmove(dst, src, sizeof(T) * count);
            }
        };

        /// \brief copy constructs count instances of T in dst from src. Dst is uninitialized
        template <class T>
        void uninitializedCopy(T* dst, const T* src, size_t count)
        {
            uninitializedCopyImpl<isPod<T>::value, T>::uninitializedCopy(dst, src, count);
        }

		//Move
		template <bool, class T>
        struct UninitializedMoveImpl
        {
            static void uninitializedMove(T* dst, T* src, size_t count)
            {
                for(size_t i=0; i<count; i++)
                    new (dst+i) T(std::move(src[i]));
            }
        };

        //pod case
        template <class T>
        struct UninitializedMoveImpl<true, T>
        {
            static void uninitializedMove(T* dst, T* src, size_t count)
            {
                if(dst + count < src || dst > src + count)
                    memcpy(dst, src, sizeof(T) * count);
                else
                    memmove(dst, src, sizeof(T) * count);
            }
        };

        /// \brief move constructs count instaces of T in dst from src. Dst is uninitialized
        template <class T>
        void UninitializedMove(T* dst, T* src, size_t count)
        {
            UninitializedMoveImpl<isPod<T>::value, T>::uninitializedMove(dst, src, count);
        }

      namespace
      {
         //non-pod case
         template<bool, class T>
         struct MoveImpl
         {
            static void move(const T* src, T* dst, size_t count)
            {
               for (size_t i = 0; i < count; i++)
               {
                  dst[i] = std::move(src[i]);
               }
            }
         };

         //pod case
         template<class T>
         struct MoveImpl<true, T>
         {
            static void move(const T* src, T* dst, size_t count)
            {
               if (dst + count < src || dst > src + count)
                  memcpy(dst, src, sizeof(T) * count);
               else
                  memmove(dst, src, sizeof(T) * count);
            }
         };

         template<typename T>
         struct DestroyTrivialImpl
         {
            static void Destroy(T* src, size_t size) {}
         };

         template<typename T>
         struct DestroyNonTrivialImpl
         {
            static void Destroy(T* src, size_t size)
            {
               for(size_t i = 0; i < size; i++)
               {
                  (src + i)->~T();
               }
            }
         };
      }

		
		/// \brief Moves count elements from src to dst
		template<class T>
		void Move(const T* src, T* dst, size_t count)
		{
			MoveImpl<std::is_trivially_move_assignable<T>::value, T>::move(src, dst, count);
		}

		//Destroy
		template<typename T>
		void Destroy(T* src, size_t size)
		{
         std::conditional_t<
            std::is_trivially_destructible_v<std::decay_t<T>>, 
		      DestroyTrivialImpl<T>, 
		      DestroyNonTrivialImpl<T>
		   >::Destroy(src, size);
		}

	}

	//----AlignmentHelper----
	struct AlignmentHelper
	{
		static const size_t ALIGNMENT = sizeof(void*);
		static size_t AlignSize(size_t s)
		{
			return ((s + ALIGNMENT - 1) / ALIGNMENT) * ALIGNMENT;
		}

		static bool IsAligned(char* ptr)
		{
			return (reinterpret_cast<uintptr_t>(ptr) % ALIGNMENT) == 0;
		}
	};

	//! \brief Returns the number of elements in the given fixed-size array
	//! \param arr Array
	//! \returns Element count of the array
	template<typename T, size_t N>
	constexpr size_t ElementCount(T(&arr)[N]) { return N; }

}

#define AlignedSize(s) pe::AlignmentHelper::AlignSize(s)