export namespace main {
	
	export class PackageInfo {
	    name: string;
	    version: string;
	    description: string;
	    repository: string;
	    maintainer: string;
	    upstreamurl: string;
	    dependlist: string[];
	    lastupdated: string;
	
	    static createFrom(source: any = {}) {
	        return new PackageInfo(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.name = source["name"];
	        this.version = source["version"];
	        this.description = source["description"];
	        this.repository = source["repository"];
	        this.maintainer = source["maintainer"];
	        this.upstreamurl = source["upstreamurl"];
	        this.dependlist = source["dependlist"];
	        this.lastupdated = source["lastupdated"];
	    }
	}
	export class UpdateInfo {
	    name: string;
	    oldVersion: string;
	    newVersion: string;
	    repository: string;
	    downloadSize: number;
	
	    static createFrom(source: any = {}) {
	        return new UpdateInfo(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.name = source["name"];
	        this.oldVersion = source["oldVersion"];
	        this.newVersion = source["newVersion"];
	        this.repository = source["repository"];
	        this.downloadSize = source["downloadSize"];
	    }
	}

}

