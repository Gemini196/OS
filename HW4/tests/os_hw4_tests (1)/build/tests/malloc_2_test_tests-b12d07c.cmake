add_test( malloc_2.Sanity [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] Sanity  )
set_tests_properties( malloc_2.Sanity PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.Check size]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[Check size]==]  )
set_tests_properties( [==[malloc_2.Check size]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.0 size]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[0 size]==]  )
set_tests_properties( [==[malloc_2.0 size]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.Max size]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[Max size]==]  )
set_tests_properties( [==[malloc_2.Max size]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( malloc_2.free [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] free  )
set_tests_properties( malloc_2.free PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.free 2]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[free 2]==]  )
set_tests_properties( [==[malloc_2.free 2]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.free 3]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[free 3]==]  )
set_tests_properties( [==[malloc_2.free 3]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( malloc_2.Reuse [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] Reuse  )
set_tests_properties( malloc_2.Reuse PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.Reuse two blocks]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[Reuse two blocks]==]  )
set_tests_properties( [==[malloc_2.Reuse two blocks]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.Reuse two blocks reverse]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[Reuse two blocks reverse]==]  )
set_tests_properties( [==[malloc_2.Reuse two blocks reverse]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.Reuse two blocks both]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[Reuse two blocks both]==]  )
set_tests_properties( [==[malloc_2.Reuse two blocks both]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.Reuse two blocks sizes small]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[Reuse two blocks sizes small]==]  )
set_tests_properties( [==[malloc_2.Reuse two blocks sizes small]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.Reuse two blocks sizes large]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[Reuse two blocks sizes large]==]  )
set_tests_properties( [==[malloc_2.Reuse two blocks sizes large]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( malloc_2.scalloc [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] scalloc  )
set_tests_properties( malloc_2.scalloc PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.scalloc taint]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[scalloc taint]==]  )
set_tests_properties( [==[malloc_2.scalloc taint]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.scalloc 0 size]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[scalloc 0 size]==]  )
set_tests_properties( [==[malloc_2.scalloc 0 size]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.scalloc Max size num]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[scalloc Max size num]==]  )
set_tests_properties( [==[malloc_2.scalloc Max size num]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.scalloc Max size size]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[scalloc Max size size]==]  )
set_tests_properties( [==[malloc_2.scalloc Max size size]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.scalloc Max size both]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[scalloc Max size both]==]  )
set_tests_properties( [==[malloc_2.scalloc Max size both]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( malloc_2.realloc [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] realloc  )
set_tests_properties( malloc_2.realloc PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.realloc shrink]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[realloc shrink]==]  )
set_tests_properties( [==[malloc_2.realloc shrink]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
add_test( [==[malloc_2.srealloc Max size]==] [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests/malloc_2_test]==] [==[srealloc Max size]==]  )
set_tests_properties( [==[malloc_2.srealloc Max size]==] PROPERTIES WORKING_DIRECTORY [==[/home/student/Desktop/OS/HW4/tests/os_hw4_tests (1)/build/tests]==])
set( malloc_2_test_TESTS malloc_2.Sanity [==[malloc_2.Check size]==] [==[malloc_2.0 size]==] [==[malloc_2.Max size]==] malloc_2.free [==[malloc_2.free 2]==] [==[malloc_2.free 3]==] malloc_2.Reuse [==[malloc_2.Reuse two blocks]==] [==[malloc_2.Reuse two blocks reverse]==] [==[malloc_2.Reuse two blocks both]==] [==[malloc_2.Reuse two blocks sizes small]==] [==[malloc_2.Reuse two blocks sizes large]==] malloc_2.scalloc [==[malloc_2.scalloc taint]==] [==[malloc_2.scalloc 0 size]==] [==[malloc_2.scalloc Max size num]==] [==[malloc_2.scalloc Max size size]==] [==[malloc_2.scalloc Max size both]==] malloc_2.realloc [==[malloc_2.realloc shrink]==] [==[malloc_2.srealloc Max size]==])
