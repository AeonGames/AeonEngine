node ('docker'){
  stage ('Configure'){
	checkout scm
	sh pwd
	sh ls
	sh 'cmake -G\\"Unix Makefiles\\" .'
  }
  stage ('Build'){
	sh 'make -j4'
  }
}
