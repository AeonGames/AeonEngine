node ('docker'){
  stage ('Configure'){
	checkout scm
	sh 'cmake -G\"Unix Makefiles .\"'
  }
  stage ('Build'){
	sh 'make -j4'
  }
}
